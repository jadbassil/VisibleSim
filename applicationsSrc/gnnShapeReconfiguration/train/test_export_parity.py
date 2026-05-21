"""
Parity test: load weights from the exported .bin with NumPy, run a forward pass
on a small synthetic graph, and compare against `policy.distributed_act` from
the Python policy. The two must agree to ~1e-5.

Usage
-----
  python test_export_parity.py --ckpt checkpoints/policy_final.pt \
                               --bin  ../../../applicationsBin/gnnLocomotion/gcn_weights.bin
"""
import argparse
import struct
import zlib

import numpy as np
import torch

from model import GNNPolicy, NODE_DIM, EDGE_DIM, HIDDEN, MAX_ACTIONS

MAGIC = b"GCN1"


def _read_array(buf: bytes, off: int, shape) -> tuple:
    n = int(np.prod(shape))
    arr = np.frombuffer(buf, dtype=np.float32, count=n, offset=off).reshape(shape)
    return arr.copy(), off + n * 4


def load_bin(path: str) -> dict:
    with open(path, "rb") as f:
        buf = f.read()
    assert buf[:4] == MAGIC, f"bad magic: {buf[:4]!r}"
    crc_stored = struct.unpack("<I", buf[-4:])[0]
    crc_actual = zlib.crc32(buf[:-4])
    assert crc_stored == crc_actual, f"crc mismatch: 0x{crc_stored:08x} != 0x{crc_actual:08x}"

    off = 4
    version = struct.unpack("<I", buf[off:off+4])[0]; off += 4
    hidden, node_dim, edge_dim, n_actions, n_layers = struct.unpack(
        "<IIIII", buf[off:off+20]); off += 20
    assert version == 1
    assert hidden == HIDDEN and node_dim == NODE_DIM and edge_dim == EDGE_DIM
    assert n_actions == MAX_ACTIONS and n_layers == 3

    layers = []
    in_dim = node_dim
    for _ in range(n_layers):
        msg_w,  off = _read_array(buf, off, (hidden, in_dim + edge_dim))
        self_w, off = _read_array(buf, off, (hidden, in_dim))
        bias,   off = _read_array(buf, off, (hidden,))
        layers.append({"msg_w": msg_w, "self_w": self_w, "bias": bias})
        in_dim = hidden

    fc1_w, off = _read_array(buf, off, (hidden, hidden))
    fc1_b, off = _read_array(buf, off, (hidden,))
    fc2_w, off = _read_array(buf, off, (n_actions, hidden))
    fc2_b, off = _read_array(buf, off, (n_actions,))

    return dict(layers=layers, fc1_w=fc1_w, fc1_b=fc1_b, fc2_w=fc2_w, fc2_b=fc2_b)


def elu(x: np.ndarray) -> np.ndarray:
    return np.where(x >= 0, x, np.exp(x) - 1.0)


def numpy_forward(weights: dict, x: np.ndarray, edge_index: np.ndarray,
                  edge_attr: np.ndarray) -> np.ndarray:
    """Replicate GCNEdgeConv: msg = msg_lin([h_j, e_ji]); out = mean_j(msg) + self_lin(h_i) + b."""
    h = x.astype(np.float32)
    src, dst = edge_index[0], edge_index[1]   # message j -> i in PyG convention

    for L in weights["layers"]:
        N = h.shape[0]
        # Aggregate
        agg = np.zeros((N, L["msg_w"].shape[0]), dtype=np.float32)
        cnt = np.zeros((N,), dtype=np.float32)
        for k in range(src.shape[0]):
            j, i = int(src[k]), int(dst[k])
            cat = np.concatenate([h[j], edge_attr[k]])
            msg = L["msg_w"] @ cat
            agg[i] += msg
            cnt[i] += 1.0
        # Mean (avoid div-by-zero for isolated nodes)
        cnt = np.maximum(cnt, 1.0)
        agg = agg / cnt[:, None]
        out = agg + h @ L["self_w"].T + L["bias"]
        h = elu(out)

    # Actor head
    a = elu(h @ weights["fc1_w"].T + weights["fc1_b"])
    logits = a @ weights["fc2_w"].T + weights["fc2_b"]
    return logits


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ckpt", required=True)
    ap.add_argument("--bin",  required=True)
    args = ap.parse_args()

    # Load PyTorch policy
    ckpt = torch.load(args.ckpt, map_location="cpu", weights_only=False)
    state = ckpt["model_state"] if "model_state" in ckpt else ckpt
    policy = GNNPolicy(backbone="gcn")
    policy.load_state_dict(state)
    policy.eval()

    # Load NumPy weights
    weights = load_bin(args.bin)

    # Synthetic 4-node graph: chain 0-1-2-3 with edges in both directions
    np.random.seed(0)
    x = np.random.randn(4, NODE_DIM).astype(np.float32)
    edges = [(0, 1), (1, 0), (1, 2), (2, 1), (2, 3), (3, 2)]
    edge_index = np.array(edges, dtype=np.int64).T   # [2, E]
    # one-hot direction of size 6 — pick a deterministic mapping: dir = k % 6
    edge_attr = np.zeros((edge_index.shape[1], EDGE_DIM), dtype=np.float32)
    for k in range(edge_attr.shape[0]):
        edge_attr[k, k % EDGE_DIM] = 1.0

    # NumPy forward
    np_logits = numpy_forward(weights, x, edge_index, edge_attr)

    # PyTorch forward (encoder + actor head, no global pooling)
    with torch.no_grad():
        h = policy.encode(torch.tensor(x),
                          torch.tensor(edge_index),
                          torch.tensor(edge_attr))
        py_logits = policy.actor_head(h).numpy()

    diff = np.abs(np_logits - py_logits).max()
    print(f"max_abs_diff = {diff:.3e}")
    assert diff < 1e-4, f"parity check failed: {diff}"
    print("[ok] export parity within tolerance")


if __name__ == "__main__":
    main()
