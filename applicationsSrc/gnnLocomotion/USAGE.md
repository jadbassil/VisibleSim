# GNN Locomotion — Usage Guide

## Prerequisites

**C++ build** (see project-root `MACOS_SETUP.md` for macOS dependencies):
```sh
cd applicationsSrc/gnnLocomotion
make -j$(sysctl -n hw.ncpu)
```

Or via CMake:
```sh
cmake -B build .
cmake --build build --target gnnLocomotion
```

**Python environment** (Python 3.9+):
```sh
cd applicationsSrc/gnnLocomotion/train
uv sync          # preferred — uses pyproject.toml + uv.lock
# or:
python -m venv .venv && source .venv/bin/activate
pip install torch numpy torch_geometric wandb
```

Without `torch_geometric` the policy degrades to an MLP that ignores graph structure.

---

## What this app does

A cluster of 6 sliding-cube modules learns to locomote as a whole in a fixed direction (default: +X). The policy is a 3-layer GNN: each module independently decides whether to move based on its local neighbourhood. The reward at each step is the displacement of the swarm's center of mass along the travel direction.

There is **no target shape** and no solve-rate criterion — the task is open-ended locomotion.

---

## Training

```sh
cd applicationsSrc/gnnLocomotion/train
source .venv/bin/activate
python train.py
```

### All options

```
python train.py [OPTIONS]

  --binary PATH       Path to VisibleSim binary
                      (default: ../../../applicationsBin/gnnLocomotion/gnnLocomotion)
  --config PATH       Path to config.xml
                      (default: ../../../applicationsBin/gnnLocomotion/config.xml)
  --port INT          TCP port for gym communication (default: 9999)
  --episodes INT      Number of training episodes (default: 2000)
  --device STR        Compute device: cpu | cuda (default: auto-detected)
  --backbone STR      GNN encoder: gat | gcn (default: gat)
  --direction DX,DY,DZ  Locomotion direction vector, will be normalised
                        (default: 1,0,0 — +X axis)
  --render            Launch VisibleSim with GUI (disabled by default)
  --realtime          With --render, use realtime scheduler for visible motion
  --step-delay FLOAT  Sleep N seconds after each step to slow playback (default: 0)
  --wandb             Enable Weights & Biases logging
  --wandb-project STR wandb project name (default: visible-sim-gnn-locomotion)
  --wandb-entity STR  wandb user/team (optional)
  --wandb-run-name STR wandb run name (optional)
  --wandb-mode STR    wandb mode: online | offline | disabled (default: online)
```

### Examples

Headless training in +X (fastest):
```sh
python train.py --episodes 2000 --backbone gat
```

Train to locomote in the +Y direction:
```sh
python train.py --direction 0,1,0
```

Train to locomote diagonally (+X +Y):
```sh
python train.py --direction 1,1,0   # normalised to [0.707, 0.707, 0] at runtime
```

Training with GUI and visible motion:
```sh
python train.py --render --realtime --step-delay 0.2
```

GCN backbone (required for deploy mode):
```sh
python train.py --backbone gcn
```

Training with wandb tracking:
```sh
python train.py --wandb --wandb-project marl-locomotion --backbone gcn
```

> **Important**: the `--direction` you pass to `train.py` must match the `direction` attribute in `config.xml` `<locomotion direction="..."/>` and `config_deploy.xml`. If they differ the directional-position feature (node x(3)) will be computed differently in Python vs. C++, and the deployed policy will behave inconsistently.

---

## Changing the Locomotion Direction

1. Edit `applicationsBin/gnnLocomotion/config.xml`:
   ```xml
   <locomotion direction="0,1,0"/>   <!-- +Y instead of +X -->
   ```
2. Pass the same direction to the training script:
   ```sh
   python train.py --direction 0,1,0
   ```
3. After training, update `config_deploy.xml` with the same direction before deploying.

No rebuild is required for direction changes — the direction is parsed at runtime.

---

## Changing the Initial Shape or Grid

Edit `applicationsBin/gnnLocomotion/config.xml`:

```xml
<world gridSize="W,H,D" windowSize="1200,800">
    <locomotion direction="dx,dy,dz"/>
    <blockList color="80,160,255" blockSize="10.0,10.0,10.0" ids="ORDERED">
        <block position="x,y,z"/>
        <!-- one entry per module -->
    </blockList>
    <!-- no targetList -->
</world>
```

Episode hyperparameters (`MAX_STEPS`, `GYM_PORT`) are constants in `gnnLocomotionBlockCode.hpp` and require a rebuild.

---

## Checkpoints

Checkpoints are saved to `train/checkpoints/`:

- `policy_ep00050.pt`, `policy_ep00100.pt`, … every 50 episodes
- `policy_final.pt` always written on exit (including Ctrl-C)

Each checkpoint contains:
```python
{
    "episode":     int,
    "model_state": dict,   # policy.state_dict()
    "optim_state": dict,   # trainer.optim.state_dict()
}
```

Loading a checkpoint:
```python
from model import GNNPolicy
import torch

policy = GNNPolicy(backbone="gat")
ckpt = torch.load("checkpoints/policy_final.pt", map_location="cpu")
policy.load_state_dict(ckpt["model_state"])
policy.eval()
```

---

## Training Output

Every 10 episodes a summary line is printed:

```
Episode    10  reward=  3.12  mean_r=  2.47  steps= 200  mean_steps=200.0
```

The reward is the total CoM displacement along the travel direction accumulated over the episode (positive = the swarm moved forward). Unlike shape reconfiguration there is no `solve_rate` column — locomotion is a continuous task.

During PPO updates:

```
  [update] loss=0.0412  pg=0.0298  vf=0.0091  ent=0.0023
```

| Field | Meaning |
|---|---|
| `reward` | Total locomotion reward this episode (ΣΔCoM·direction) |
| `mean_r` | Mean episode reward over last 10 episodes |
| `steps` | Always `MAX_STEPS` unless a connectivity violation ends the episode early |
| `loss` | Total PPO loss |
| `pg` | Policy gradient loss |
| `vf` | Value function loss |
| `ent` | Mean entropy (higher = more exploration) |

---

## Running the Simulator Standalone (without Python)

To inspect the initial configuration visually:
```sh
cd applicationsBin/gnnLocomotion
./gnnLocomotion -c config.xml -r    # GUI, realtime
./gnnLocomotion -c config.xml -t    # headless (blocks waiting for gym client)
```

Without a Python client connected the simulator waits for a step request. Use Ctrl-C to exit.

---

## Distributed Inference (on-robot deployment)

`GNNPolicy.distributed_act()` runs the encoder and actor head only, with no global pooling. Each physical module can execute it independently after exchanging `L=3` rounds of messages with its neighbours:

```python
policy = GNNPolicy(backbone="gcn")
policy.load_state_dict(torch.load("checkpoints/policy_final.pt")["model_state"])
policy.eval()

# x, edge_index, edge_attr built from local neighbourhood only
# x[:, 3] = directionalPos = dot(pos, direction) / grid_extent  (not in_target)
probs = policy.distributed_act(x, edge_index, edge_attr, action_masks)
action = probs.argmax(dim=-1)
```

The GCN backbone is preferred for on-robot use because the per-hop information radius is explicit and the parameter count is lower.

---

## Deploy Mode (C++ in-simulator distributed inference)

The simulator can run the trained policy directly in C++ — no Python loop, no TCP. Each module independently runs the GCN forward pass on its local 3-hop neighbourhood and decides its own motion every step.

### 1. Train a GCN policy

```sh
cd applicationsSrc/gnnLocomotion/train
source .venv/bin/activate
python train.py --backbone gcn --episodes 2000 --direction 1,0,0
```

Deploy mode requires the GCN backbone (the C++ runtime is GCN-only).

### 2. Export weights to the binary format

```sh
python export_weights.py \
  --ckpt checkpoints/policy_final.pt \
  --out  ../../../applicationsBin/gnnLocomotion/gcn_weights.bin
```

This writes `gcn_weights.bin` and a sibling `gcn_weights.bin.json` listing the tensor shapes.

### 3. Verify export parity (recommended)

```sh
python test_export_parity.py \
  --ckpt checkpoints/policy_final.pt \
  --bin  ../../../applicationsBin/gnnLocomotion/gcn_weights.bin
```

The script loads the binary with NumPy, runs forward on a synthetic 4-node graph, and asserts `max_abs_diff < 1e-4` against the PyTorch policy.

### 4. Run the simulator in deploy mode

`applicationsBin/gnnLocomotion/config_deploy.xml` enables deploy mode:

```xml
<deploy enabled="true" weights="gcn_weights.bin" seed="42"/>
<locomotion direction="1,0,0"/>
```

Run it:

```sh
cd applicationsBin/gnnLocomotion
./gnnLocomotion -c config_deploy.xml -r        # GUI, realtime
./gnnLocomotion -c config_deploy.xml -t        # headless, fastest scheduler
```

You should see one log line per module per step:

```
[GCN id=1 step=1 action=2 dp=0.17 isAP=0]
[GCN id=2 step=1 action=0 dp=0.25 isAP=1]
...
```

- `dp` — directional position `dot(pos, direction) / extent` for this module
- `isAP=1` — this module is an articulation point and was forced to stay

The simulation terminates at `MAX_STEPS` (no shape-completion check).

### 5. Override weights without editing XML

```sh
GNN_DEPLOY_WEIGHTS=/absolute/path/to/gcn_weights.bin \
  ./gnnLocomotion -c config.xml -t
```

### Known limitations

- **Feature drift**: as the swarm locomotes, `directionalPos` values grow. If the swarm travels beyond the grid extent used during training, this feature saturates at or above 1.0, which is out-of-distribution. Use a longer grid or re-train with domain randomisation over starting positions.
- **Local AP detection** uses the 2-hop induced subgraph; it is exact for clusters smaller than the GNN's 3-hop receptive field (≤ ~20 modules in dense lattices).
- **One mover per step**: the ballot selects one mover per step, mirroring the training protocol. This limits locomotion speed. A top-k ballot extension would improve throughput.
