# GNN Locomotion — Self-Reconfiguration via Reinforcement Learning

## Overview

This application trains a Graph Neural Network (GNN) policy with PPO reinforcement learning to solve **modular robot self-reconfiguration**: a set of SlidingCubes modules must locomote across a 3D grid and arrange themselves into a target shape. VisibleSim acts as the physics environment; a Python process acts as the trainer. They communicate over a TCP socket.

---

## Architecture

```
┌─────────────────────────────────────────┐     TCP socket      ┌─────────────────────────────┐
│           VisibleSim (C++)              │   (port 9999, JSON) │      Python trainer         │
│                                         │ ◄──────────────────► │                             │
│  Scheduler thread                       │                      │  env.py  VisibleSimEnv      │
│  ├─ GNNLocomotionCode (all blocks)      │   initial obs ──►    │  ├─ reset() → launch proc   │
│  │   ├─ startup()  [leader only]        │   ◄── step req       │  ├─ step() → send actions   │
│  │   ├─ onGymTick() [leader]            │   obs+reward ──►     │  └─ obs_to_tensors()        │
│  │   ├─ executeNextMove() [leader]      │                      │                             │
│  │   └─ onMotionEnd() [all blocks]      │                      │  model.py  GNNPolicy        │
│  │                                      │                      │  ├─ 3-layer GATConv encoder  │
│  └─ GymServer (background thread)       │                      │  ├─ per-node actor head      │
│      ├─ accept() → wait for Python      │                      │  └─ global mean-pool critic  │
│      ├─ send initial obs on connect     │                      │                             │
│      ├─ recv step → signal scheduler    │                      │  ppo.py  PPOTrainer         │
│      └─ wait for obs → send to Python   │                      │  ├─ GAE advantage estimation │
│                                         │                      │  ├─ clipped PPO update       │
└─────────────────────────────────────────┘                      │  └─ action masking           │
                                                                  │                             │
                                                                  │  train.py  main loop        │
                                                                  └─────────────────────────────┘
```

---

## File Reference

### C++ — `applicationsSrc/gnnLocomotion/`

| File | Responsibility |
|---|---|
| `gnnLocomotion.cpp` | `main()` — wires `buildNewBlockCode` into `SlidingCubes::createSimulator` |
| `gnnLocomotionBlockCode.hpp/.cpp` | Distributed block code. The **leader** (lowest block ID) drives the gym protocol. All blocks report motion completion via `onMotionEnd()`. |
| `gymServer.hpp/.cpp` | TCP socket server in a background thread. Serialises observations as newline-delimited JSON; parses action messages from Python. |

### Python — `applicationsSrc/gnnLocomotion/train/`

| File | Responsibility |
|---|---|
| `env.py` | `VisibleSimEnv` — launches the binary as a subprocess, speaks the JSON protocol, converts observations into graph tensors |
| `model.py` | `GNNPolicy` — 3-layer GAT encoder, per-node actor head, global-mean-pool critic head |
| `ppo.py` | `PPOTrainer` — rollout buffer, GAE advantage estimation, clipped PPO update with action masking |
| `train.py` | Training loop — episodes, logging, checkpointing |

---

## C++ Design Details

### Leader block state machine

```
startup()
  └─ start GymServer, post initial obs, scheduleGymTick()

onGymTick()  [fires every TICK_IDLE_US = 5 ms until a step arrives]
  ├─ no step yet → reschedule tick
  └─ step arrived → executeActions(actions)

executeActions()
  └─ fill moveQueue with (block, destPos) pairs sorted by block ID
     └─ executeNextMove()

executeNextMove()  [also called on MOVE_DONE interruption]
  ├─ queue not empty → block.moveTo(dest)
  │     if valid   → return and wait for MOVE_DONE
  │     if invalid → skip (collision), try next entry
  └─ queue empty → onAllMovesComplete()

onAllMovesComplete()
  └─ compute reward, build obs, GymServer::setObs(), scheduleGymTick()
```

### Thread safety

`onMotionEnd()` (scheduler thread) calls `GymServer::notifyMoveDone()`, which fires a callback that schedules an `InterruptionEvent<int>(MOVE_DONE)` back onto the leader block. The move queue is always driven from the scheduler thread — no mutex is needed for the queue itself.

The two cross-thread signals are:
- **Python → scheduler**: `bool stepReady` polled non-blockingly inside `onGymTick()`.
- **Scheduler → Python**: `std::condition_variable cv_obs` with `bool obsReady` predicate, so the socket thread blocks until the step resolves.

---

## JSON Protocol

```
C++ → Python  (on connect and after every step):
{
  "type": "obs",
  "step": 5,
  "max_steps": 500,
  "reward": 0.5,
  "done": false,
  "grid_size": [10, 10, 4],
  "blocks": [
    { "id": 1, "pos": [1,1,0], "in_target": false,
      "neighbors": [-1, 2, -1, -1, -1, -1],
      "moves": [[2,1,0],[1,2,0]] }
  ],
  "target": [[6,6,0],[6,7,0],[6,8,0],[7,6,0],[8,6,0],[8,7,0]]
}

Python → C++  (each step):
{"type":"step","actions":{"1":0,"2":1,"3":2,"4":0,"5":1,"6":0}}
```

`neighbors` is a 6-element array indexed by direction (PlusX=0, PlusY=1, PlusZ=2, MinusX=3, MinusY=4, MinusZ=5); value is the neighbour's block ID or `-1` if unoccupied.

Action index `0` = stay; `1..N` = move to `getAllMotions()[index-1]` destination.

---

## GNN Policy

### Node features (11-dim per block)

| Index | Feature |
|---|---|
| 0–2 | `x, y, z` normalised by grid size |
| 3 | `in_target` (0 or 1) |
| 4–9 | Neighbour-presence bits per direction (0 or 1) |
| 10 | `n_moves / 12` (normalised move count) |

### Edge features (6-dim)

One-hot connector direction (PlusX..MinusZ).

### Network

```
x [N,11] ──► GATConv(11→64, heads=4) ──► ELU
          ──► GATConv(64→64, heads=4) ──► ELU
          ──► GATConv(64→64, heads=1) ──► ELU
                │                               │
         actor_head                       global mean-pool
         FC(64→64)→ELU→FC(64,13)          FC(64→64)→ELU→FC(64,1)
                │                               │
         logits [N,13]                    value scalar
    (masked to -∞ for invalid actions)
```

Falls back to a plain MLP encoder if `torch_geometric` is not installed.

### Action masking

`action_masks[i][j] = True` when action `j` is valid for block `i`. Invalid logits are set to `-inf` before the categorical distribution is sampled, so the policy never selects out-of-range moves.

---

## PPO Hyperparameters

| Parameter | Value |
|---|---|
| Learning rate | 3e-4 |
| Discount γ | 0.99 |
| GAE λ | 0.95 |
| Clip ε | 0.2 |
| Value loss coefficient | 0.5 |
| Entropy coefficient | 0.01 |
| Rollout steps before update | 256 |
| PPO epochs per rollout | 4 |
| Mini-batch size | 32 |

---

## Reward Function

| Event | Reward |
|---|---|
| Each block newly placed in target | +1.0 |
| Per step (time penalty) | −0.01 |
| Episode completion (all target cells filled) | +10.0 |

---

## Configuration — `applicationsBin/gnnLocomotion/config.xml`

- **Grid**: 10×10×4
- **Initial shape**: 2×3 rectangle at (1–3, 1–2, z=0)
- **Target shape**: L-shape at (6–8, 6–8, z=0)
- **6 modules**, 6 target cells

---

## Build & Run

### 1. Build the simulator

Ensure the VisibleSim core libraries are already built, then:

```bash
# Make build
cd applicationsSrc/gnnLocomotion
make

# CMake build (from project root)
cmake -B build .
make -j$(sysctl -n hw.ncpu) -C build gnnLocomotion
```

The binary is placed at `applicationsBin/gnnLocomotion/gnnLocomotion`.

### 2. Install Python dependencies

```bash
cd applicationsSrc/gnnLocomotion/train
pip install -r requirements.txt
```

`torch_geometric` is optional but strongly recommended for the full GAT-based GNN. Without it, the policy falls back to a plain MLP.

### 3. Train

```bash
cd applicationsSrc/gnnLocomotion/train
python train.py
```

Optional arguments:

```
--binary   PATH      Path to the gnnLocomotion binary  (default: auto-resolved)
--config   PATH      Path to config.xml                (default: auto-resolved)
--port     INT       TCP port for gym server            (default: 9999)
--episodes INT       Number of training episodes        (default: 2000)
--device   cpu|cuda  PyTorch device                     (default: cuda if available)
```

Checkpoints are saved to `train/checkpoints/policy_ep<N>.pt` every 50 episodes and as `policy_final.pt` on exit.

### 4. Test the socket manually

Run the simulator in the background, then connect from Python:

```bash
./applicationsBin/gnnLocomotion/gnnLocomotion -c applicationsBin/gnnLocomotion/config.xml -R &
```

```python
import socket, json

s = socket.socket()
s.connect(('127.0.0.1', 9999))
obs = json.loads(s.makefile().readline())
print(f"{len(obs['blocks'])} blocks, {len(obs['target'])} target cells")

# Send all-stay action
actions = {str(b['id']): 0 for b in obs['blocks']}
s.sendall((json.dumps({"type": "step", "actions": actions}) + "\n").encode())
result = json.loads(s.makefile().readline())
print(f"reward={result['reward']:.3f}  done={result['done']}")
```

### 5. Load a checkpoint for inference

```python
import torch
from model import GNNPolicy

policy = GNNPolicy()
ckpt = torch.load('train/checkpoints/policy_final.pt')
policy.load_state_dict(ckpt['model_state'])
policy.eval()
```

---

## Episode Lifecycle

```
Python                          C++ (VisibleSim)
──────                          ────────────────
env.reset()
  └─ launch subprocess
  └─ socket.connect(9999)  ──►  accept()
                                startup() → GymServer::setObs(initial_obs)
                           ◄──  send initial obs JSON

env.step(actions)
  └─ send step JSON        ──►  stepReady = true
                                onGymTick() → executeActions()
                                  moves execute sequentially
                                  onAllMovesComplete() → setObs(new_obs)
                           ◄──  send obs+reward JSON

  ... repeat until done=true ...

  └─ env.close()
      └─ kill subprocess
      └─ socket.close()
```