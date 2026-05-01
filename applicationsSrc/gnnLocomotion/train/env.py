"""
VisibleSimEnv — Gym wrapper around the gnnLocomotion VisibleSim binary.

Protocol (newline-delimited JSON over TCP):
  C++ → Python:  observation JSON  (on connect and after every step)
  Python → C++:  {"type":"step","actions":{"<blockId>": <actionIdx>, ...}}

Action index 0 = stay; 1..N = index into getAllMotions() for that block.
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
NODE_FEAT_DIM = 12          # 3 pos + 1 in_target + 6 neighbor-presence bits + 1 n_moves + 1 is_ap


class VisibleSimEnv:
    """
    Single-process Gym-style environment backed by a VisibleSim subprocess.

    reset() launches the binary and returns the initial observation dict.
    step(actions_dict) sends actions and returns (obs, reward, done, info).
    close() kills the subprocess and closes the socket.

    Graph tensors for the GNN are produced by obs_to_tensors(obs).
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
    ):
        self.binary_path = os.path.abspath(binary_path)
        self.config_path = os.path.abspath(config_path)
        self.port = port
        self.render = render
        self.realtime = realtime
        self.connect_timeout = connect_timeout
        self.step_timeout = step_timeout

        self.proc: Optional[subprocess.Popen] = None
        self.sock: Optional[socket.socket] = None
        self.block_ids: List[int] = []  # ordered block IDs from last obs
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

        # Potential-based shaping: F = γ·Φ(s') − Φ(s).  Using γ=1 (no discount).
        # Φ(s) = −(mean min-distance from each non-target block to nearest target cell)
        # This gives +reward when blocks move closer to target cells.
        new_potential = self._potential(obs)
        shaping = new_potential - self._prev_potential
        self._prev_potential = new_potential

        reward = obs["reward"] + 0.1 * shaping
        return obs, reward, obs["done"], {"step": obs["step"]}

    def close(self):
        self._close()

    # ------------------------------------------------------------------
    # Graph tensor conversion
    # ------------------------------------------------------------------

    def obs_to_tensors(self, obs: dict):
        """
        Returns (x, edge_index, edge_attr, action_masks) as torch tensors.

        x            : [N, NODE_FEAT_DIM]  node features
        edge_index   : [2, E]              directed edges (both directions)
        edge_attr    : [E, 6]              one-hot direction per edge
        action_masks : [N, MAX_ACTIONS]    bool; True = action is valid

        Articulation-point constraint: blocks whose removal would disconnect
        the cluster have all non-stay actions masked out.
        """
        blocks = obs["blocks"]
        grid   = obs["grid_size"]

        id_to_idx = {b["id"]: i for i, b in enumerate(blocks)}

        # Compute articulation points once; used for both features and masks.
        art_points = self._articulation_points(obs)

        node_feats = []
        for b in blocks:
            feats = [
                b["pos"][0] / max(grid[0], 1),
                b["pos"][1] / max(grid[1], 1),
                b["pos"][2] / max(grid[2], 1),
                float(b["in_target"]),
                *[float(nb != -1) for nb in b["neighbors"]],   # 6 bits
                min(len(b["moves"]), MAX_ACTIONS_PER_BLOCK - 1) / (MAX_ACTIONS_PER_BLOCK - 1),
                float(b["id"] in art_points),                  # is_ap flag
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

        # Action mask: action 0 (stay) always valid; actions 1..len(moves) valid.
        # Articulation points and already-placed blocks are forced to stay.
        masks = []
        for b in blocks:
            if b["id"] in art_points or b["in_target"]:
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
        """
        Return the set of block IDs that are articulation points (cut vertices):
        removing them would disconnect the remaining cluster.
        Uses a simple BFS reachability check for each candidate block.
        """
        blocks  = obs["blocks"]
        all_ids = {b["id"] for b in blocks}
        if len(all_ids) <= 2:
            return set()  # trivially connected regardless

        # Adjacency list from the observation neighbor data
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
        Φ(s) = −mean over non-target blocks of the L1 distance to the
               nearest target cell.  More negative = farther from goal.
        Shaping reward F = Φ(s') − Φ(s) is positive when blocks move closer.
        """
        targets = obs.get("target", [])
        if not targets:
            return 0.0
        target_arr = np.array([[t[0], t[1], t[2]] for t in targets], dtype=np.float32)

        non_target_blocks = [b for b in obs["blocks"] if not b["in_target"]]
        if not non_target_blocks:
            return 0.0

        total = 0.0
        for b in non_target_blocks:
            pos = np.array(b["pos"], dtype=np.float32)
            dists = np.abs(target_arr - pos).sum(axis=1)
            total += float(dists.min())

        return -(total / len(non_target_blocks))

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _launch(self):
        sim_cwd = os.path.dirname(self.binary_path)
        cmd = [self.binary_path, "-c", self.config_path]
        if self.render:
            # In GUI mode, choose realtime for visible animation or fastest
            # for best throughput.
            cmd.append("-r" if self.realtime else "-R")
        else:
            # Use -t (terminal/headless mode) to skip GLUT initialisation.
            # -t implies -R (fastest scheduler); gym-tick events keep the
            # event queue non-empty so the scheduler never exits prematurely.
            cmd.append("-t")

        self.proc = subprocess.Popen(
            cmd,
            cwd=sim_cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            preexec_fn=os.setsid,   # create new process group for clean kill
        )
        deadline = time.time() + self.connect_timeout
        while time.time() < deadline:
            # Check if the process crashed before we connected
            if self.proc.poll() is not None:
                out = self.proc.stdout.read().decode(errors="replace")
                raise RuntimeError(
                    f"VisibleSim exited (code {self.proc.returncode}) "
                    f"before accepting connections:\n{out}"
                )
            try:
                # Create a fresh socket each attempt — on macOS a failed
                # connect() leaves the socket unusable for reconnection.
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
        # Collect any output to help diagnose the failure
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
