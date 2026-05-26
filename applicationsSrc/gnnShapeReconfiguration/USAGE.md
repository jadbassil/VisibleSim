# GNN Shape Reconfiguration — Usage Guide

## Prerequisites

**C++ build** (see project-root `MACOS_SETUP.md` for macOS dependencies):

```sh
cd applicationsSrc/gnnShapeReconfiguration
make -j$(sysctl -n hw.ncpu)
```

Or via CMake from the project root:

```sh
cmake -B build .
cmake --build build --target gnnShapeReconfiguration
```

**Python environment** (Python 3.9+):

```sh
cd applicationsSrc/gnnShapeReconfiguration/train
uv sync          # preferred — uses pyproject.toml + uv.lock
# or:
python -m venv .venv && source .venv/bin/activate
pip install torch numpy torch_geometric wandb
```

Without `torch_geometric` the policy degrades to an MLP that ignores graph structure.

---

## What this app does

A cluster of sliding-cube modules learns to reconfigure itself into a target shape specified in `config.xml` (via `<targetList>`). The policy is a 3-layer GNN: each module independently decides whether to move based on its local neighbourhood. The reward at each step combines a per-block target-placement signal (computed in C++) with a global potential-shaping term (computed in Python from L1 distance to the target).

Training terminates either after `--episodes` iterations or when the rolling solve-rate hits 95% over a 200-episode window.

---

## Training

```sh
cd applicationsSrc/gnnShapeReconfiguration/train
source .venv/bin/activate         # or use `uv run` in front of the command
python train.py
```

### All options

```
python train.py [OPTIONS]

  --binary PATH       Path to VisibleSim binary
                      (default: ../../../applicationsBin/gnnShapeReconfiguration/gnnShapeReconfiguration)
  --config PATH       Path to config.xml
                      (default: ../../../applicationsBin/gnnShapeReconfiguration/config.xml)
  --port INT          TCP port for gym communication (default: 9999)
  --episodes INT      Number of training episodes (default: 2000)
  --device STR        Compute device: cpu | cuda (default: auto-detected)
  --backbone STR      GNN encoder: gat | gcn (default: gat)
  --render            Launch VisibleSim with GUI (disabled by default)
  --realtime          With --render, use realtime scheduler for visible motion
  --step-delay FLOAT  Sleep N seconds after each step to slow playback (default: 0)
  --wandb             Enable Weights & Biases logging
  --wandb-project STR wandb project name (default: visible-sim-gnn-shape-reconfiguration)
  --wandb-entity STR  wandb user/team (optional)
  --wandb-run-name STR wandb run name (optional)
  --wandb-mode STR    wandb mode: online | offline | disabled (default: online)
```

### Examples

Headless training, default GAT backbone (fastest):

```sh
python train.py --episodes 2000
```

GCN backbone — required if you plan to deploy:

```sh
python train.py --backbone gcn --episodes 2000
```

Training with GUI and visible motion (good for debugging):

```sh
python train.py --render --realtime --step-delay 0.2
```

Training with wandb tracking:

```sh
python train.py --wandb --wandb-project marl-shape --backbone gcn
```

---

## Changing the Target Shape or Initial Configuration

Edit `applicationsBin/gnnShapeReconfiguration/config.xml`:

```xml
<world gridSize="W,H,D" windowSize="1200,800">
    <blockList color="80,160,255" blockSize="10.0,10.0,10.0" ids="ORDERED">
        <block position="x,y,z"/>     <!-- one entry per module -->
        ...
    </blockList>

    <targetList>
        <target format="grid">
            <cell position="x,y,z" color="255,100,100"/>   <!-- one entry per target cell -->
            ...
        </target>
    </targetList>
</world>
```

The number of `<block>` entries should match the number of `<cell>` entries for the episode to be solvable. No rebuild is required for shape changes — the XML is parsed at runtime.

Episode hyperparameters (`MAX_STEPS`, `GYM_PORT`) are constants in `gnnShapeReconfigurationBlockCode.hpp` and require a rebuild.

---

## Checkpoints

Checkpoints are saved to `train/checkpoints/`:

- `policy_ep00050.pt`, `policy_ep00100.pt`, … every 50 episodes
- `policy_final.pt` always written on exit (including Ctrl-C and early stop)

Each checkpoint contains:

```python
{
    "episode":     int,
    "model_state": dict,   # policy.state_dict()
    "optim_state": dict,   # trainer.optim.state_dict()
}
```

Loading a checkpoint in Python:

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
Episode    10  reward=  3.12  mean_r=  2.47  steps= 184  mean_steps=192.0
Episode   200  reward= 12.20  mean_r=  9.84  steps=  72  mean_steps= 91.4  solve_rate=0.78
```

| Field | Meaning |
|---|---|
| `reward` | Total episode reward (sum of `R_sim + 0.1·ΔΦ`) |
| `mean_r` | Mean episode reward over last 10 episodes |
| `steps` | Number of gym steps taken before the episode ended |
| `mean_steps` | Mean over last 10 episodes |
| `solve_rate` | Fraction of the last 200 episodes that filled all target cells |

During PPO updates:

```
  [update] loss=0.0412  pg=0.0298  vf=0.0091  ent=0.0023
```

| Field | Meaning |
|---|---|
| `loss` | Total PPO loss |
| `pg` | Clipped policy-gradient loss |
| `vf` | Value function loss |
| `ent` | Mean entropy (higher = more exploration) |

---

## Reward Reference

Per step the trainer receives:

```
R_total = R_sim + 0.1 · (Φ(s') − Φ(s))
```

where, with `n_in = countBlocksInTarget()`:

| Component | Source | Value |
|---|---|---|
| `+1` per newly placed block | C++ | `n_in(s') − n_in(s)` |
| `−1` per block that leaves | C++ | (same term, sign-aware) |
| `−0.01` step cost | C++ | constant |
| `+10` shape-complete bonus | C++ | only on terminating success |
| `−10` connectivity penalty | C++ | overrides others; episode ends |
| `0.1·ΔΦ` distance shaping | Python | `Φ(s) = −mean L1 dist to nearest target cell` |

Total expected episode reward when fully solved is roughly `+10 (bonus) + N (per-placement) − 0.01·steps + shaping`.

---

## Action Space and Masking Reference

Per block, 13 discrete actions:

| Index | Action |
|---|---|
| `0` | Stay |
| `1..k` | Move to `motions[i-1]` (from `SlidingCubesBlock::getAllMotions()`) |
| `k+1..12` | Unavailable for this block — masked out |

The mask passed to the policy enforces:

1. **Articulation-point lock** — removing the block would split the cluster ⇒ only `stay` is allowed.
2. **In-target lock** — the block already sits on a target cell ⇒ only `stay` is allowed.
3. **Available-moves bound** — actions beyond the number of valid motions are masked.

The C++ side re-checks the articulation condition before committing each move; a violation triggers the `−10` penalty and ends the episode.

---

## Running the Simulator Standalone (without Python)

To inspect the initial configuration visually:

```sh
cd applicationsBin/gnnShapeReconfiguration
./gnnShapeReconfiguration -c config.xml -r    # GUI, realtime
./gnnShapeReconfiguration -c config.xml -t    # headless (blocks waiting for gym client)
```

Without a Python client connected the simulator waits for a step request. Use Ctrl-C to exit. To exercise the trained policy without Python, use deploy mode below.

---

## Distributed Inference (Python preview)

`GNNPolicy.distributed_act()` runs the encoder and actor head only, with no global pooling. Each physical module can execute it independently after exchanging `L = 3` rounds of messages with its neighbours:

```python
policy = GNNPolicy(backbone="gcn")
policy.load_state_dict(torch.load("checkpoints/policy_final.pt")["model_state"])
policy.eval()

# x, edge_index, edge_attr built from local neighbourhood only
# x[:, 3] = in_target  (1.0 if the block occupies a target cell)
probs = policy.distributed_act(x, edge_index, edge_attr, action_masks)
action = probs.argmax(dim=-1)
```

The GCN backbone is preferred for on-robot use because the per-hop information radius is explicit and the parameter count is lower.

---

## Deploy Mode (C++ in-simulator distributed inference)

The simulator can run the trained policy directly in C++ — no Python loop, no TCP. Each module independently runs the GCN forward pass on its local 3-hop neighbourhood and decides its own motion every step.

### 1. Train a GCN policy

```sh
cd applicationsSrc/gnnShapeReconfiguration/train
source .venv/bin/activate
python train.py --backbone gcn --episodes 2000
```

Deploy mode requires the GCN backbone (the C++ runtime is GCN-only).

### 2. Export weights to the binary format

```sh
python export_weights.py \
  --ckpt checkpoints/policy_final.pt \
  --out  ../../../applicationsBin/gnnShapeReconfiguration/gcn_weights.bin
```

This writes `gcn_weights.bin` and a sibling `gcn_weights.bin.json` listing the tensor shapes.

### 3. Verify export parity (recommended)

```sh
python test_export_parity.py \
  --ckpt checkpoints/policy_final.pt \
  --bin  ../../../applicationsBin/gnnShapeReconfiguration/gcn_weights.bin
```

The script loads the binary with NumPy, runs forward on a synthetic 4-node graph, and asserts `max_abs_diff < 1e-4` against the PyTorch policy.

### 4. Run the simulator in deploy mode

`applicationsBin/gnnShapeReconfiguration/config_deploy.xml` enables deploy mode:

```xml
<deploy enabled="true" weights="gcn_weights.bin" seed="42"/>
```

Run it:

```sh
cd applicationsBin/gnnShapeReconfiguration
./gnnShapeReconfiguration -c config_deploy.xml -r        # GUI, realtime
./gnnShapeReconfiguration -c config_deploy.xml -t        # headless, fastest scheduler
```

You should see one log line per module per step:

```
[GCN id=1 step=1 action=2 inTarget=0 isAP=0]
[GCN id=2 step=1 action=0 inTarget=0 isAP=1]
...
[GCN deploy] target shape reached at step 47 (6/6 in target)
```

- `inTarget=1` — module already on a target cell; action was forced to `stay`.
- `isAP=1` — module is an articulation point; action was forced to `stay`.

The simulation terminates on shape completion or when `deployStep >= MAX_STEPS`.

### 5. Override weights without editing XML

```sh
GNN_DEPLOY_WEIGHTS=/absolute/path/to/gcn_weights.bin \
  ./gnnShapeReconfiguration -c config.xml -t
```

`GNN_DEPLOY_WEIGHTS` activates deploy mode even when `<deploy enabled="...">` is absent or false.

---

## Distributed Inference Mechanism (deploy mode summary)

Each step, every module runs the following local sequence — no leader, no shared state except a small in-process ballot used to pick the single mover for this step (a stand-in for a physical-layer arbitration that would be needed on real hardware):

1. **Topology phase (2 rounds of P2P messages)**
   - Round 1 (`MSG_TOPO_1HOP`): broadcast own id + neighbour bitmask to all 6 face neighbours.
   - Round 2 (`MSG_TOPO_2HOP`): broadcast collected 1-hop info. After this round every module knows its 2-hop induced subgraph.
   - Locally compute `is_ap` (articulation point) via BFS over the 2-hop subgraph excluding self.
   - Read `in_target` from the local target predicate.
   - Construct the 12-dim node feature `h⁰`.

2. **GNN message-passing phase (3 rounds, one per GCN layer)**
   - For layer `L ∈ {1, 2, 3}`: send `msg_lin([h^{L-1} ‖ e_ji])` to each neighbour over its physical interface.
   - On receipt of all neighbour messages (or watchdog expiry), aggregate via mean, add `self_lin(h^{L-1})` and bias, apply ELU → `h^L`.

3. **Action phase (local actor head)**
   - Compute `logits = actor_head(h³)`.
   - Apply the same mask used during training: `stay` always allowed; `move 1..k` allowed only if `!in_target && !is_ap`.
   - Take `argmax` of masked logits.
   - Check the proposed destination against the last 4 visited positions; if matched, suppress to `stay` (anti-oscillation).

4. **Ballot / arbitration**
   - Each module registers `{id, action, motions}` in a shared `DeployBallot`.
   - The last voter elects the lowest-id non-stay action as the **winner** and issues exactly one `moveTo()` per step.
   - The winner pushes its current position into its history.
   - If every module voted stay (all blocked by history), all histories are cleared.
   - The last voter schedules the next `DEPLOY_STEP_START` for all modules at the same future simulation time, allowing motion to complete first (`max(motionDuration, 8 × ROUND_DT_US)`).

5. **Termination check**
   - `countBlocksInTarget() ≥ |target|` → success → no more steps scheduled.
   - `deployStep ≥ MAX_STEPS` → step budget exhausted → no more steps scheduled.

Each phase has a watchdog `InterruptionEvent` firing `3 × ROUND_DT_US` after the phase begins, guarded against firing if the phase has already advanced (so stale watchdogs from a previous step are dropped). The watchdogs make the protocol robust when a neighbour is mid-motion and cannot reply.

### Known limitations

- **2-hop AP detection** uses the 2-hop induced subgraph; it is exact for clusters smaller than the GNN's 3-hop receptive field (≤ ~20 modules in dense lattices).
- **One mover per step**: the ballot selects one mover per step, mirroring the training protocol. This limits reconfiguration speed; a top-k ballot extension would improve throughput.
- **In-target lock at deploy time**: same as training. A placed block never moves again even if a better solution would briefly displace it.
