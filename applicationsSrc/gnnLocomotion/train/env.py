"""
VisibleSimEnv — Gym wrapper around the gnnLocomotion VisibleSim binary.

Protocol (newline-delimited JSON over TCP):
  C++ → Python:  observation JSON  (on connect and after every step)
  Python → C++:  {"type":"step","actions":{"<blockId>": <actionIdx>, ...}}

Action index 0 = stay; 1..N = index into getAllMotions() for that block.

Locomotion objective: maximise displacement of the swarm's center of mass
along a fixed direction vector.  Reward = Φ(s') − Φ(s) where
Φ(s) = dot(CoM, direction).  There is no terminal shape condition.
"""
import json
import os
import signal
import socket
import subprocess
import time
from typing import Dict, List, Optional, Tuple

import numpy as np
import torch

MAX_ACTIONS_PER_BLOCK = 13  # 0=stay + up to 12 moves
NODE_FEAT_DIM = 12          # 3 pos + 1 directional_pos + 6 neighbor bits + 1 n_moves + 1 is_ap


class VisibleSimEnv:
    """
    Single-process Gym-style environment backed by a VisibleSim subprocess.

    Parameters
    ----------
    direction : (3,) array-like
        Locomotion direction unit vector. Defaults to +X = [1, 0, 0].
    """

    def __init__(
        self,
        binary_path: str,
        config_path: str,
        port: int = 9999,
        render: bool = False,
        realtime: bool = True,
        connect_timeout: float = 20.0,
        step_timeout: float = 30.0,
        direction: Tuple[float, float, float] = (1.0, 0.0, 0.0),
    ):
        self.binary_path = os.path.abspath(binary_path)
        self.config_path = os.path.abspath(config_path)
        self.port = port
        self.render = render
        self.realtime = realtime
        self.connect_timeout = connect_timeout
        self.step_timeout = step_timeout

        d = np.array(direction, dtype=np.float32)
        norm = np.linalg.norm(d)
        self.direction = d / norm if norm > 1e-6 else np.array([1.0, 0.0, 0.0], dtype=np.float32)

        self.proc: Optional[subprocess.Popen] = None
        self.sock: Optional[socket.socket] = None
        self.block_ids: List[int] = []
        self._prev_potential: float = 0.0

    # ------------------------------------------------------------------
    # Core interface
    # ------------------------------------------------------------------

    def reset(self) -> dict:
        self._close()
        self._launch()
        obs = self._recv_obs()
        self.block_ids = [b["id"] for b in obs["blocks"]]
        self._prev_potential = self._potential(obs)
        return obs

    def step(self, actions: Dict[int, int]) -> Tuple[dict, float, bool, dict]:
        """
        actions: {block_id: action_index}  (action 0 = stay)
        """
        msg = json.dumps({"type": "step",
                          "actions": {str(k): int(v) for k, v in actions.items()}})
        self._send(msg)
        obs = self._recv_obs()
        self.block_ids = [b["id"] for b in obs["blocks"]]

        # Locomotion reward: ΔCoM · direction (potential shaping, γ=1).
        # obs["reward"] carries the connectivity penalty (-10) if applicable;
        # otherwise it is 0.
        new_potential = self._potential(obs)
        shaping = new_potential - self._prev_potential
        self._prev_potential = new_potential

        reward = obs["reward"] + shaping
        return obs, reward, obs["done"], {"step": obs["step"]}

    def close(self):
        self._close()

    # ------------------------------------------------------------------
    # Graph tensor conversion
    # ------------------------------------------------------------------

    def obs_to_tensors(self, obs: dict):
        """
        Returns (x, edge_index, edge_attr, action_masks) as torch tensors.

        x(3) = directional_pos = dot(pos, direction) / grid_max_extent
               replaces the in_target flag from shape reconfiguration.
        """
        blocks = obs["blocks"]
        grid   = obs["grid_size"]

        id_to_idx = {b["id"]: i for i, b in enumerate(blocks)}
        art_points = self._articulation_points(obs)

        # Grid extent along the travel direction for normalisation.
        grid_arr = np.array(grid, dtype=np.float32)
        extent = float(np.dot(self.direction, grid_arr))
        if extent < 1.0:
            extent = 1.0

        node_feats = []
        for b in blocks:
            pos = np.array(b["pos"], dtype=np.float32)
            directional_pos = float(np.dot(self.direction, pos)) / extent
            feats = [
                b["pos"][0] / max(grid[0], 1),
                b["pos"][1] / max(grid[1], 1),
                b["pos"][2] / max(grid[2], 1),
                directional_pos,                                            # replaces in_target
                *[float(nb != -1) for nb in b["neighbors"]],               # 6 bits
                min(len(b["moves"]), MAX_ACTIONS_PER_BLOCK - 1) / (MAX_ACTIONS_PER_BLOCK - 1),
                float(b["id"] in art_points),
            ]
            node_feats.append(feats)

        edge_src, edge_dst, edge_attr = [], [], []
        for i, b in enumerate(blocks):
            for d, nb_id in enumerate(b["neighbors"]):
                if nb_id != -1 and nb_id in id_to_idx:
                    j = id_to_idx[nb_id]
                    edge_src.append(i)
                    edge_dst.append(j)
                    one_hot = [0.0] * 6
                    one_hot[d] = 1.0
                    edge_attr.append(one_hot)

        # Locomotion mask: only articulation points are forced to stay.
        # There are no in_target locked blocks.
        masks = []
        for b in blocks:
            if b["id"] in art_points:
                mask = [True] + [False] * (MAX_ACTIONS_PER_BLOCK - 1)
            else:
                n_moves = min(len(b["moves"]), MAX_ACTIONS_PER_BLOCK - 1)
                mask = [True] * (n_moves + 1) + [False] * (MAX_ACTIONS_PER_BLOCK - 1 - n_moves)
            masks.append(mask)

        x = torch.tensor(node_feats, dtype=torch.float32)

        if edge_src:
            edge_index = torch.tensor([edge_src, edge_dst], dtype=torch.long)
            edge_attr_t = torch.tensor(edge_attr, dtype=torch.float32)
        else:
            edge_index  = torch.zeros((2, 0), dtype=torch.long)
            edge_attr_t = torch.zeros((0, 6), dtype=torch.float32)

        action_masks = torch.tensor(masks, dtype=torch.bool)

        return x, edge_index, edge_attr_t, action_masks

    # ------------------------------------------------------------------
    # Connectivity helpers
    # ------------------------------------------------------------------

    def _articulation_points(self, obs: dict) -> set:
        blocks  = obs["blocks"]
        all_ids = {b["id"] for b in blocks}
        if len(all_ids) <= 2:
            return set()

        adj: Dict[int, List[int]] = {
            b["id"]: [nb for nb in b["neighbors"] if nb != -1 and nb in all_ids]
            for b in blocks
        }

        def connected_without(removed: int) -> bool:
            remaining = all_ids - {removed}
            if not remaining:
                return True
            start = next(iter(remaining))
            visited = {start}
            queue   = [start]
            while queue:
                node = queue.pop()
                for nb in adj[node]:
                    if nb in remaining and nb not in visited:
                        visited.add(nb)
                        queue.append(nb)
            return visited == remaining

        return {b["id"] for b in blocks if not connected_without(b["id"])}

    def _potential(self, obs: dict) -> float:
        """
        Φ(s) = dot(CoM, direction).
        Shaping reward F = Φ(s') − Φ(s) = dot(ΔCoM, direction):
        positive when the swarm moves along the travel axis.
        """
        positions = np.array([b["pos"] for b in obs["blocks"]], dtype=np.float32)
        com = positions.mean(axis=0)
        return float(np.dot(self.direction, com))

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _launch(self):
        sim_cwd = os.path.dirname(self.binary_path)
        cmd = [self.binary_path, "-c", self.config_path]
        if self.render:
            cmd.append("-r" if self.realtime else "-R")
        else:
            cmd.append("-t")

        self.proc = subprocess.Popen(
            cmd,
            cwd=sim_cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            preexec_fn=os.setsid,
        )
        deadline = time.time() + self.connect_timeout
        while time.time() < deadline:
            if self.proc.poll() is not None:
                out = self.proc.stdout.read().decode(errors="replace")
                raise RuntimeError(
                    f"VisibleSim exited (code {self.proc.returncode}) "
                    f"before accepting connections:\n{out}"
                )
            try:
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.sock.connect(("127.0.0.1", self.port))
                self.sock.settimeout(self.step_timeout)
                return
            except (ConnectionRefusedError, OSError):
                try:
                    self.sock.close()
                except OSError:
                    pass
                self.sock = None
                time.sleep(0.05)
        self.proc.terminate()
        out = self.proc.stdout.read().decode(errors="replace")
        raise TimeoutError(
            f"Could not connect to VisibleSim on port {self.port} "
            f"within {self.connect_timeout}s.\nSimulator output:\n{out}"
        )

    def _close(self):
        if self.sock:
            try:
                self.sock.close()
            except OSError:
                pass
            self.sock = None
        if self.proc:
            try:
                os.killpg(os.getpgid(self.proc.pid), signal.SIGTERM)
                self.proc.wait(timeout=2)
            except Exception:
                try:
                    self.proc.kill()
                except Exception:
                    pass
            self.proc = None

    def _send(self, msg: str):
        self.sock.sendall((msg + "\n").encode())

    def _recv_obs(self) -> dict:
        buf = b""
        while True:
            chunk = self.sock.recv(65536)
            if not chunk:
                raise ConnectionError("VisibleSim disconnected unexpectedly")
            buf += chunk
            if b"\n" in buf:
                line, _ = buf.split(b"\n", 1)
                return json.loads(line.decode())
