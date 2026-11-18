"""
Export a trained GCN policy to a flat little-endian binary for the C++ deploy runtime.

Usage
-----
  python export_weights.py --ckpt checkpoints/policy_final.pt \
                           --out  ../../../applicationsBin/gnnShapeReconfiguration/gcn_weights.bin
"""
import argparse
import json
import os
import struct
import zlib

import numpy as np
import torch

from model import GNNPolicy, NODE_DIM, EDGE_DIM, HIDDEN, MAX_ACTIONS

MAGIC   = b"GCN1"
VERSION = 1
N_LAYERS = 3


def _f32_bytes(t: torch.Tensor) -> bytes:
    return t.detach().cpu().contiguous().numpy().astype(np.float32).tobytes()


def export(ckpt_path: str, out_path: str) -> None:
    ckpt = torch.load(ckpt_path, map_location="cpu", weights_only=False)
    state = ckpt["model_state"] if "model_state" in ckpt else ckpt

    policy = GNNPolicy(backbone="gcn")
    policy.load_state_dict(state)
    policy.eval()

    payload = bytearray()
    payload += MAGIC
    payload += struct.pack("<I", VERSION)
    payload += struct.pack("<IIIII", HIDDEN, NODE_DIM, EDGE_DIM, MAX_ACTIONS, N_LAYERS)

    shapes: list = []
    convs = [policy.conv1, policy.conv2, policy.conv3]
    for i, conv in enumerate(convs):
        msg_w  = conv.msg_lin.weight
        self_w = conv.self_lin.weight
        bias   = conv.bias

        payload += _f32_bytes(msg_w)
        payload += _f32_bytes(self_w)
        payload += _f32_bytes(bias)

        shapes.append({
            "layer":           i,
            "msg_lin.weight":  list(msg_w.shape),
            "self_lin.weight": list(self_w.shape),
            "bias":            list(bias.shape),
        })

    fc1 = policy.actor_head[0]
    fc2 = policy.actor_head[2]
    payload += _f32_bytes(fc1.weight)
    payload += _f32_bytes(fc1.bias)
    payload += _f32_bytes(fc2.weight)
    payload += _f32_bytes(fc2.bias)
    shapes.append({
        "actor_fc1.weight": list(fc1.weight.shape),
        "actor_fc1.bias":   list(fc1.bias.shape),
        "actor_fc2.weight": list(fc2.weight.shape),
        "actor_fc2.bias":   list(fc2.bias.shape),
    })

    crc = zlib.crc32(bytes(payload))
    payload += struct.pack("<I", crc)

    out_path = os.path.abspath(out_path)
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "wb") as f:
        f.write(payload)

    meta = {
        "magic":      MAGIC.decode(),
        "version":    VERSION,
        "hidden":     HIDDEN,
        "node_dim":   NODE_DIM,
        "edge_dim":   EDGE_DIM,
        "n_actions":  MAX_ACTIONS,
        "n_layers":   N_LAYERS,
        "crc32":      crc,
        "size_bytes": len(payload),
        "shapes":     shapes,
    }
    with open(out_path + ".json", "w") as f:
        json.dump(meta, f, indent=2)

    print(f"[export] wrote {out_path}  ({len(payload)} bytes, crc32=0x{crc:08x})")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ckpt", required=True, help="path to policy_final.pt")
    ap.add_argument("--out",  required=True, help="path to output .bin")
    args = ap.parse_args()
    export(args.ckpt, args.out)


if __name__ == "__main__":
    main()
