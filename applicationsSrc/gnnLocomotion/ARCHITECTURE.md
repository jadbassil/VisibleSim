# GNN Locomotion — Architecture

## Overview

This application trains a Graph Neural Network (GNN) policy to control a cluster of modular sliding-cube robots to reconfigure from an initial shape into a target shape. The C++ simulator (VisibleSim) acts as the physics engine and is controlled by a Python training loop over a TCP socket.

---

## System Components

```
┌─────────────────────────┐        TCP (localhost)        ┌──────────────────────────┐
│   Python Training Loop  │ ◄────────────────────────────► │  VisibleSim (C++ process) │
│                         │   JSON over newline-delimited  │                           │
│  env.py  VisibleSimEnv  │        stream                  │  GymServer (socket thread) │
│  model.py  GNNPolicy    │                                │  GNNLocomotionBlockCode    │
│  ppo.py  PPOTrainer     │                                │  SlidingCubes simulator    │
└─────────────────────────┘                                └──────────────────────────┘
```

---

## Python–VisibleSim Communication Protocol

Communication uses **newline-delimited JSON over a persistent TCP connection** (default port 9999).

### Connection lifecycle

1. Python (`VisibleSimEnv.reset()`) spawns the VisibleSim binary as a subprocess.
2. Python retries `socket.connect()` until VisibleSim accepts (up to `connect_timeout` seconds).
3. A new socket object is created on each retry — required on macOS where a failed `connect()` leaves the socket unusable.
4. VisibleSim sends the **initial observation** immediately after the client connects.
5. The connection persists for the entire episode; Python kills the process on `close()`.

### Message format

**C++ → Python (observation)**

Sent at episode start and after every step:

```json
{
  "step": 5,
  "max_steps": 200,
  "grid_size": [8, 6, 4],
  "reward": 0.99,
  "done": false,
  "blocks": [
    {
      "id": 1,
      "pos": [1, 1, 0],
      "in_target": false,
      "neighbors": [2, -1, -1, -1, -1, -1],
      "moves": [[2, 1, 0], [1, 2, 0]]
    }
  ],
  "target": [[3,1,0], [4,1,0], [5,1,0], [3,2,0], [4,2,0], [5,2,0]]
}
```

- `neighbors[d]` — block ID of the neighbor in direction `d` (PlusX, MinusX, PlusY, MinusY, PlusZ, MinusZ), or `-1` if empty.
- `moves` — list of valid destination positions from `getAllMotions()`.

**Python → C++ (step)**

```json
{"type": "step", "actions": {"1": 2, "2": 0, "3": 1, "4": 0, "5": 3, "6": 0}}
```

- Action `0` = stay; actions `1..N` are 1-indexed into the block's `moves` list.

### Threading model (C++ side)

`GymServer` runs a dedicated **socket thread** that blocks on I/O. The VisibleSim **scheduler thread** drives the simulation. They communicate via a shared mutex:

- Scheduler calls `GymServer::hasStep()` (non-blocking) on each `GYM_TICK` interruption event.
- Socket thread calls `GymServer::setObs()` to unblock; scheduler reads obs via `cv_obs`.
- Move completion is signalled via `notifyMoveDone()` → `InterruptionEvent<MOVE_DONE>` on the scheduler.

---

## Simulation Step Execution

One **gym step** corresponds to exactly **one block moving**, never more. This is an intentional design choice:

1. Python sends a dict of `{block_id: action}` for all blocks.
2. C++ enqueues valid moves in ascending block-ID order.
3. Only the first move that passes the **articulation-point check** is executed.
4. The rest of the queue is discarded; Python receives the updated state and chooses again.

This eliminates intermediate connectivity states that would never be visible to the policy.

### Connectivity enforcement (dual-layer)

| Layer | Where | What |
|---|---|---|
| Python mask | `env.py: _articulation_points()` | BFS on observation snapshot; masks all moves for cut vertices |
| C++ guard | `gnnLocomotionBlockCode.cpp: isArticulationPoint()` | BFS on live lattice before each `moveTo()`; violation → penalty −10 + episode ends |

The C++ check catches cases where the Python snapshot is stale (e.g., a previous move in the same step changed connectivity).

---

## Reward Function

The total step reward received by Python combines two signals:

```
R_total = R_sim + 0.1 × (Φ(s') − Φ(s))
```

**R_sim** (from C++):
- `+1` for each block newly placed in a target cell
- `−0.01` time penalty per step
- `+10` completion bonus when all target cells are filled
- `−10` connectivity-violation penalty (episode ends)

**Potential-based shaping** (Python, `env.py: _potential()`):
- `Φ(s) = −mean L1 distance from each non-target block to its nearest target cell`
- Shaping `F = Φ(s') − Φ(s)` is positive whenever any block moves closer to the goal
- Coefficient `0.1` keeps shaping subordinate to the discrete placement signal

---

## Action Masking

Before passing tensors to the policy, `obs_to_tensors()` builds a boolean mask `[N, 13]`:

| Condition | Mask |
|---|---|
| Block is an articulation point | `[True, False×12]` — stay only |
| Block is already in a target cell | `[True, False×12]` — stay only |
| Normal block with `k` valid moves | `[True×(k+1), False×(12−k)]` |

Masking is applied inside the policy (`logits.masked_fill(~mask, −∞)`) before softmax.

---

## GNN Policy (`model.py`)

### Architecture

```
Input: node features [N, 12], edge features [E, 6]
  │
  ├── Encoder: 3 × GNN layer (GAT or GCN) → [N, 64]
  │
  ├── Actor head:  Linear(64,64) → ELU → Linear(64,13) → logits [N, 13]  (per node)
  └── Critic head: global mean-pool → Linear(64,64) → ELU → Linear(64,1) → scalar
```

### Node features (12 dims)

| Feature | Dims | Description |
|---|---|---|
| Normalised position | 3 | `pos / grid_size` |
| In-target flag | 1 | `1.0` if block is already placed |
| Neighbour presence | 6 | One bit per direction (PlusX…MinusZ) |
| Normalised move count | 1 | `n_moves / 12` |
| Articulation-point flag | 1 | `1.0` if removing this block disconnects the cluster |

The articulation-point flag lets the policy **internalise the connectivity constraint**: during training the model learns to correlate `is_ap=1` with the stay action, removing the need for an external mask at inference time. At inference on physical robots, each module computes its own AP flag using its L-hop neighbourhood (exact for clusters smaller than the GNN's receptive field).

### Edge features (6 dims)

One-hot encoding of the connector direction (PlusX … MinusZ).

### Backbone options

**GAT** (`backbone='gat'`, default): Graph Attention Network. Each layer computes attention weights over neighbours. Richer representation; recommended for training.

**GCN** (`backbone='gcn'`): `GCNEdgeConv` — mean aggregation with edge features concatenated into the message:

```
msg_{j→i}  = W_msg · [h_j ‖ e_{ji}]
h_i'       = mean_{j∈N(i)}(msg_{j→i}) + W_self · h_i + b
```

Lower parameter count; the hop-count / information-radius relationship is explicit. Recommended for analysis and on-robot deployment.

### Distributed inference property

After `L` message-passing layers, each node's hidden state depends only on its `L`-hop neighbourhood. `distributed_act()` runs encoder + actor only (no global pooling), so each physical module can execute it independently after `L` rounds of local message exchange with neighbours.

The articulation-point feature is the only input that requires graph-wide knowledge. Each module computes it locally by checking whether its own removal disconnects its known L-hop subgraph. For clusters smaller than the receptive field (e.g., ≤~20 modules with L=3 and branching factor ≤6) this local check is exact.

---

## PPO Training (`ppo.py`)

| Hyperparameter | Value |
|---|---|
| Learning rate | 3 × 10⁻⁴ |
| Rollout length | 20 steps (before each update) |
| Epochs per update | 4 |
| Mini-batch size | 32 transitions |
| Clip ε | 0.2 |
| Value coefficient | 0.5 |
| Entropy coefficient | 0.01 |
| GAE λ | 0.95 |
| Discount γ | 0.99 |

Advantages are computed with **Generalized Advantage Estimation (GAE)** and normalised globally across the rollout. The value function is a single global scalar (centralised critic); advantages are broadcast to all nodes in the graph. The policy gradient uses the **clipped surrogate objective** (standard PPO). Per-node log-probabilities are averaged over all nodes for the policy loss.

---

## Episode Configuration (`config.xml`)

The default curriculum stage uses a 2×3 rectangle that must locomote 2 cells to the right:

- **Initial shape**: blocks at `(1..3, 1..2, 0)` — 6 modules
- **Target shape**: cells at `(3..5, 1..2, 0)` — shifted right by 2
- **Overlap**: 2 blocks (at x=3) start already inside the target, providing an immediate positive reward signal on episode start
- **Max steps**: 200 per episode
- **Grid**: 8×6×4
