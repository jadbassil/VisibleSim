# GNN Locomotion — Usage Guide

## Prerequisites

**C++ build** (see project-root `MACOS_SETUP.md` for macOS dependencies):
```sh
# From project root
cmake -B build .
cmake --build build --target gnnLocomotion
```

**Python environment** (Python 3.9+):
```sh
cd applicationsSrc/gnnLocomotion/train
python -m venv .venv
source .venv/bin/activate
pip install torch numpy
pip install torch_geometric   # optional but recommended — enables GAT/GCN backbones
pip install wandb             # optional — experiment tracking
```

Without `torch_geometric` the policy degrades to an MLP that ignores graph structure.

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

Headless training (fastest):
```sh
python train.py --episodes 2000 --backbone gat
```

Training with GUI and visible motion:
```sh
python train.py --render --realtime --step-delay 0.2
```

GCN backbone (lighter, easier to analyse):
```sh
python train.py --backbone gcn
```

Training with wandb tracking:
```sh
python train.py --wandb --wandb-project marl --backbone gcn
```

Training with wandb in offline mode:
```sh
python train.py --wandb --wandb-mode offline --wandb-project marl
```

### Weights & Biases (wandb)

If `--wandb` is set, the trainer logs:

- PPO update metrics (`loss`, `pg`, `vf`, `ent`, step reward stats)
- Episode metrics (reward, steps, rolling means, target reached)
- Checkpoint events

If `wandb` is not installed, training continues and prints a warning.

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
Episode    10  reward=  2.40  mean_r=  1.85  steps=  47  mean_steps=52.3
```

During PPO updates (also every 10 episodes):

```
  [update] loss=0.0423  pg=0.0311  vf=0.0089  ent=0.0023  r_step=0.980  r_step_mean=0.712  ep_r=2.40
```

| Field | Meaning |
|---|---|
| `reward` | Total reward for this episode |
| `mean_r` | Mean episode reward over last 10 episodes |
| `steps` | Steps taken this episode |
| `loss` | Total PPO loss |
| `pg` | Policy gradient loss |
| `vf` | Value function loss |
| `ent` | Mean entropy (higher = more exploration) |

---

## Changing the Task

Edit `applicationsBin/gnnLocomotion/config.xml` to change the initial and target shapes, then rebuild:

```sh
cmake --build build --target gnnLocomotion
```

The XML format:
```xml
<world gridSize="W,H,D" windowSize="1200,800">
    <blockList color="80,160,255" blockSize="10.0,10.0,10.0" ids="ORDERED">
        <block position="x,y,z"/>
        <!-- one entry per module -->
    </blockList>
    <targetList>
        <target format="grid">
            <cell position="x,y,z" color="255,100,100"/>
        </target>
    </targetList>
</world>
```

Episode hyperparameters (`MAX_STEPS`, `GYM_PORT`) are constants in `gnnLocomotionBlockCode.hpp` and require a rebuild.

---

## Running the Simulator Standalone (without Python)

To inspect the initial configuration visually:
```sh
cd applicationsBin/gnnLocomotion
./gnnLocomotion -c config.xml -r    # GUI, realtime
./gnnLocomotion -c config.xml -t    # headless (exits immediately without gym client)
```

Without a Python client connected the simulator will block waiting for a step request. Use Ctrl-C to exit.

---

## Distributed Inference (on-robot deployment)

`GNNPolicy.distributed_act()` runs the encoder and actor head only, with no global pooling. Each physical module can execute it independently after exchanging `L=3` rounds of messages with its neighbours:

```python
policy = GNNPolicy(backbone="gcn")
policy.load_state_dict(torch.load("checkpoints/policy_final.pt")["model_state"])
policy.eval()

# x, edge_index, edge_attr built from local neighbourhood only
probs = policy.distributed_act(x, edge_index, edge_attr, action_masks)
action = probs.argmax(dim=-1)
```

The GCN backbone is preferred for on-robot use because the per-hop information radius is explicit and the parameter count is lower (≈28k vs ≈38k for GAT).

---

## Deploy Mode (C++ in-simulator distributed inference)

The simulator can run the trained policy directly in C++ — no Python loop, no leader, no TCP. Each module independently runs the GCN forward pass on its local 3-hop neighbourhood and decides its own motion every step. See `ARCHITECTURE.md` for the protocol details.

### 1. Train a GCN policy

```sh
cd applicationsSrc/gnnLocomotion/train
source .venv/bin/activate
python train.py --backbone gcn --episodes 2000
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

`applicationsBin/gnnLocomotion/config_deploy.xml` enables deploy mode via:

```xml
<deploy enabled="true" weights="gcn_weights.bin" seed="42"/>
```

Run it:

```sh
# From project root, after building gnnLocomotion:
cd applicationsBin/gnnLocomotion
./gnnLocomotion -c config_deploy.xml -r        # GUI, realtime
./gnnLocomotion -c config_deploy.xml -t        # headless, fastest scheduler
```

You should see one log line per module per step:

```
[GCN id=1 step=1 action=2 inTarget=0 isAP=0]
[GCN id=2 step=1 action=0 inTarget=0 isAP=1]
...
```

`moveTo` failures (collisions when two modules pick the same destination) are logged but the simulation continues.

### 5. Override weights without editing XML

```sh
GNN_DEPLOY_WEIGHTS=/absolute/path/to/gcn_weights.bin \
  ./gnnLocomotion -c config.xml -t
```

The env var sets `deployMode = true` regardless of the XML element, which is convenient for batch experiments and CI.

### Known limitations

- All modules move simultaneously every step, diverging from the one-move-per-step protocol used during training. Expect a success-rate gap until a simultaneous-move fine-tune is added.
- Local articulation-point detection uses the 2-hop induced subgraph; it is exact for clusters smaller than the GNN's 3-hop receptive field (≤ ~20 modules in dense lattices).
