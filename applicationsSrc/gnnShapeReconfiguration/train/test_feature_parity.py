"""
Feature-construction parity test.

test_export_parity.py checks the forward-pass math on random features; it does
NOT check that env.py and the C++ deploy side build the SAME node features.
This test closes that gap for the target-relative features (distance/direction
to nearest unfilled target).

It runs the C++ dumper (featureParity, built with `make feature-parity`) and
compares its per-block 16-d vectors against env.VisibleSimEnv.obs_to_tensors on
the identical hand-built scenarios.

Scenarios use <= 2 blocks so the articulation-point set is empty on both sides.
A target cell is "filled" iff an in_target block sits on it, which is exactly how
env.py derives occupancy -- so the scenarios below are kept internally consistent.

Usage
-----
  cd applicationsSrc/gnnShapeReconfiguration && make feature-parity
  cd train && python test_feature_parity.py
"""
import json
import os
import subprocess

import numpy as np

import env

BINARY = os.path.normpath(os.path.join(
    os.path.dirname(__file__),
    "../../../applicationsBin/gnnShapeReconfiguration/featureParity"))

# Must mirror the scenarios hard-coded in featureParity.cpp.
SCENARIOS = [
    {
        "id": 1,
        "grid": [6, 6, 6],
        "target": [[1, 0, 0], [2, 0, 0], [3, 0, 0], [0, 2, 0]],
        "blocks": [
            {"name": "A", "pos": [0, 0, 0], "in_target": False, "n_moves": 2},
            {"name": "B", "pos": [1, 0, 0], "in_target": True,  "n_moves": 0},
        ],
    },
    {
        "id": 2,
        "grid": [4, 4, 4],
        "target": [[3, 3, 3]],
        "blocks": [
            {"name": "A", "pos": [0, 0, 0], "in_target": False, "n_moves": 1},
        ],
    },
    {
        "id": 3,
        "grid": [5, 5, 5],
        "target": [[0, 0, 0]],
        "blocks": [
            {"name": "A", "pos": [0, 0, 0], "in_target": True, "n_moves": 0},
        ],
    },
]


def build_obs(scn: dict) -> dict:
    """Turn a scenario into the obs dict env.obs_to_tensors expects."""
    blocks = []
    for i, b in enumerate(scn["blocks"]):
        blocks.append({
            "id": i,
            "pos": b["pos"],
            "in_target": b["in_target"],
            "neighbors": [-1, -1, -1, -1, -1, -1],   # isolated (mask6 == 0)
            "moves": [[0, 0, 0]] * b["n_moves"],      # only the count matters
        })
    return {"grid_size": scn["grid"], "blocks": blocks, "target": scn["target"]}


def python_features() -> dict:
    """{(scenario_id, block_name): np.array[16]} from env.obs_to_tensors."""
    e = env.VisibleSimEnv.__new__(env.VisibleSimEnv)  # bypass __init__/socket
    out = {}
    for scn in SCENARIOS:
        obs = build_obs(scn)
        x, *_ = env.VisibleSimEnv.obs_to_tensors(e, obs)
        for i, b in enumerate(scn["blocks"]):
            out[(scn["id"], b["name"])] = x[i].numpy()
    return out


def cpp_features() -> dict:
    if not os.path.exists(BINARY):
        raise SystemExit(
            f"dumper not found at {BINARY}\n"
            "build it first: (cd .. && make feature-parity)")
    res = subprocess.run([BINARY], capture_output=True, text=True, check=True)
    out = {}
    for line in res.stdout.splitlines():
        line = line.strip()
        if not line:
            continue
        rec = json.loads(line)
        out[(rec["scenario"], rec["block"])] = np.array(rec["x"], dtype=np.float32)
    return out


def main():
    py = python_features()
    cpp = cpp_features()

    assert set(py) == set(cpp), (
        f"scenario/block mismatch:\n  py = {sorted(py)}\n  cpp= {sorted(cpp)}")

    worst = 0.0
    for key in sorted(py):
        diff = float(np.abs(py[key] - cpp[key]).max())
        worst = max(worst, diff)
        tag = "ok " if diff < 1e-5 else "BAD"
        print(f"[{tag}] scenario {key[0]} block {key[1]}: max_abs_diff = {diff:.3e}")

    print(f"\nworst max_abs_diff = {worst:.3e}")
    assert worst < 1e-5, f"feature parity failed: {worst}"
    print("[ok] feature construction parity within tolerance")


if __name__ == "__main__":
    main()
