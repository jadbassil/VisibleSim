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

---

## Distributed Deployment (deploy mode)

Deploy mode runs the trained GCN policy directly inside the simulator with **no Python loop, no leader, and no central coordinator**. Each module loads the same weights file at startup, runs its own copy of the forward pass in C++ (Eigen), and decides its own motion every step. This is a faithful simulation of true on-robot execution; the same C++ code is portable to physical hardware.

### Components

- **`train/export_weights.py`** — exports the trained `GNNPolicy(backbone='gcn')` weights to a flat little-endian binary (CRC-32 verified). Header: magic `GCN1`, version, `hidden=64`, `node_dim=12`, `edge_dim=6`, `n_actions=13`, `n_layers=3`. Body: 3 layers × (`msg_lin.weight`, `self_lin.weight`, `bias`), then actor `fc1`/`fc2` weights and biases, then a CRC-32 trailer.
- **`gcnPolicy.hpp` / `.cpp`** — pure C++ Eigen port of the GCN forward pass. Singleton `GCNWeights::instance()` loads once; all modules share the read-only copy. Helpers: `buildNodeFeature(...)`, `gcnEmitMsg`, `gcnLayerForward`, `actorLogits`, `argmaxMasked`, `sampleMaskedSoftmax`.
- **`gnnLocomotionBlockCode` (deploy path)** — added handlers, message types, and per-step state machine alongside the existing leader/gym path. Activated by a `<deploy>` element in `config.xml` or the `GNN_DEPLOY_WEIGHTS` env var.

### Per-step protocol

Each step executes a fixed sequence of P2P rounds, scheduled via `InterruptionEvent` and `t0 + ROUND_DT_US` deltas (default `2000 µs`). Every payload carries `step` so stale messages from prior steps are dropped.

```
DEPLOY_STEP_START  (interruption)
  ├─ Phase A round 1   MSG_TOPO_1HOP    broadcast 1-hop neighbour mask + id
  ├─ Phase A round 2   MSG_TOPO_2HOP    broadcast neighbour ids → each node has 2-hop subgraph
  │                   ↳ compute is_ap locally via reachability over 2-hop induced subgraph
  │                   ↳ build 12-d node feature h⁰
  ├─ Phase B round 1   MSG_GNN_LAYER1   send msg_lin([h⁰ ‖ e_ji]) to each neighbour → h¹
  ├─ Phase B round 2   MSG_GNN_LAYER2   → h²
  ├─ Phase B round 3   MSG_GNN_LAYER3   → h³
  └─ Phase C           actorLogits(h³) → masked argmax → moveTo() (or stay)
```

A round advances when either every connected interface has delivered its message for that round (the common case) or a watchdog/empty-buffer guard fires. Modules whose neighbour just moved tolerate missing messages: an isolated node treats the aggregated term as zero.

### Message payloads

| Type id | Name              | Payload struct                                            |
|---------|-------------------|-----------------------------------------------------------|
| 100     | `MSG_TOPO_1HOP`   | `{ step, id, mask6 }`                                     |
| 101     | `MSG_TOPO_2HOP`   | `{ step, id, mask6, nbIds[6] }`                           |
| 200     | `MSG_GNN_LAYER1`  | `{ step, fromDir, h[64] }` (msg_lin([h⁰ ‖ e_ji]))         |
| 201     | `MSG_GNN_LAYER2`  | `{ step, fromDir, h[64] }`                                |
| 202     | `MSG_GNN_LAYER3`  | `{ step, fromDir, h[64] }`                                |

`fromDir` is the direction that the *receiver* sees the sender on (i.e., `d ^ 1` from the sender's perspective; SCLattice2 directions are paired Plus/Minus per axis). This avoids sending the full 6-d edge feature on the wire.

### Local articulation-point detection

Each module's 2-hop induced subgraph (built from `MSG_TOPO_2HOP` messages) is enough to test whether removing this module disconnects its direct neighbours from one another. The procedure:

1. Build adjacency over `{ self ∪ direct neighbours ∪ 2-hop ids }` from the topology messages.
2. Pick any direct neighbour as BFS source; run BFS *excluding* self.
3. If any direct neighbour is unreachable from the source, declare self an articulation point.

This is exact when every cut vertex has a witness within 2 hops. For pathological skinny clusters where the witness lies further away, the deployed policy falls back on its training-time bias (the `is_ap` feature was a node input during training, so the model still leans toward staying when the local context is a "bridge").

### Action selection

The mask used at deploy time mirrors the training-time mask:

- action `0` (stay) always valid;
- actions `1..k` valid for the `k` motions returned by `getAllMotions()`;
- if the local AP test or the in-target flag is true, force stay.

Action is picked by `argmaxMasked` for deterministic deployment. `sampleMaskedSoftmax` is also available for stochastic execution; a per-block xorshift64* RNG is seeded from `globalSeed ^ blockId` for reproducibility.

### Step pacing and motion

After acting, each module schedules the next `DEPLOY_STEP_START` at `now + 8 × ROUND_DT_US`, providing headroom for any in-flight motion. `onMotionEnd()` clears `motionPending`; if a step fires while a motion is still in flight, the step is deferred by one round.

### Configuration

```xml
<world ...>
  <deploy enabled="true" weights="gcn_weights.bin" seed="42"/>
  <blockList ...> ... </blockList>
  <targetList ...> ... </targetList>
</world>
```

`weights` is resolved relative to the simulator's current working directory (i.e., `applicationsBin/gnnLocomotion/`). The env var `GNN_DEPLOY_WEIGHTS=<absolute_path>` overrides the XML setting and is convenient for batch scripts.

### Known limitations

- **All modules act simultaneously**, which diverges from the one-move-per-step protocol used during training. Expect a measurable success-rate gap. Mitigations (future work): a fine-tune pass with simultaneous moves, or a confidence-based mover-selection round (extra message exchanging max-softmax magnitudes; only top-k blocks move per step).
- **2-hop AP detection** is exact only when every articulation point has a witness within 2 hops. Acceptable for the training cluster sizes (≤ ~20 modules); larger clusters would need extra topology rounds.
- **Round delays** are simulation conveniences (`t0 + ROUND_DT_US`); they do not model real wireless contention.
