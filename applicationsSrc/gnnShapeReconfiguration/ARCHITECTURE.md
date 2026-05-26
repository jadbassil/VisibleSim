# GNN Shape Reconfiguration — Architecture

## Overview

This application trains a Graph Neural Network (GNN) policy to drive a cluster of modular sliding-cube robots from an initial configuration to a **user-specified target shape**. Each module independently decides whether to move and where, guided by a per-block reward signal (placement in target cells) combined with a global potential-shaping term (mean distance to nearest target cell).

The C++ simulator (VisibleSim) acts as the physics engine and is controlled by a Python training loop over a TCP socket. After training, the same GCN weights can be deployed directly inside the simulator with no Python process — every module runs its own copy of the forward pass in C++ and elects a single mover per step.

> **Relationship to `gnnLocomotion`**: both applications share the same distributed GNN infrastructure (gym protocol, GCN forward pass, PPO loop, ballot mechanism). They differ in objective (target shape vs. CoM translation), node feature x(3), action mask, and reward.

---

## System Components

```
┌─────────────────────────┐        TCP (localhost)        ┌──────────────────────────┐
│   Python Training Loop  │ ◄────────────────────────────► │  VisibleSim (C++ process) │
│                         │   JSON over newline-delimited  │                           │
│  env.py  VisibleSimEnv  │        stream                  │  GymServer (socket thread) │
│  model.py  GNNPolicy    │                                │  GNNShapeReconfiguration   │
│  ppo.py  PPOTrainer     │                                │  SlidingCubes simulator    │
└─────────────────────────┘                                └──────────────────────────┘
```

---

## Python–VisibleSim Communication Protocol

Communication uses **newline-delimited JSON over a persistent TCP connection** (default port `9999`).

### Connection lifecycle

1. Python (`VisibleSimEnv.reset()`) spawns the VisibleSim binary as a subprocess.
2. Python retries `socket.connect()` until VisibleSim accepts (up to `connect_timeout` seconds).
3. A new socket object is created on each retry — required on macOS where a failed `connect()` leaves the socket unusable.
4. VisibleSim sends the **initial observation** immediately after the client connects.
5. The connection persists for the entire episode; Python kills the process on `close()`.

### Message format

**C++ → Python (observation)** — sent at episode start and after every step:

```json
{
  "step": 5,
  "max_steps": 200,
  "grid_size": [8, 6, 4],
  "reward": 0.0,
  "done": false,
  "blocks": [
    {
      "id": 1,
      "pos": [3, 1, 0],
      "in_target": true,
      "neighbors": [2, -1, -1, -1, -1, -1],
      "moves": [[4, 1, 0], [3, 2, 0]]
    }
  ],
  "target": [[3,1,0],[4,1,0],[3,1,1],[4,1,1],[3,1,2],[4,1,2]]
}
```

- `reward` — per-block target placement reward computed in C++ (`Δin_target − 0.01 + 10·done`). Connectivity violations override it with `−10`.
- `in_target` — `true` if the block currently occupies a cell of the target shape.
- `target` — flat list of all target-shape cells (constant for the episode).
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
2. C++ enqueues the requested moves in ascending block-ID order.
3. Only the first move that passes the **articulation-point check** is executed.
4. The rest of the queue is discarded; Python receives the updated state and chooses again.

This eliminates intermediate connectivity states that would never be visible to the policy.

### Connectivity enforcement (dual-layer)

| Layer | Where | What |
|---|---|---|
| Python mask | `env.py: _articulation_points()` | BFS on observation snapshot; masks all moves for cut vertices |
| C++ guard | `gnnShapeReconfigurationBlockCode.cpp: isArticulationPoint()` | BFS on live lattice before each `moveTo()`; violation → penalty `−10` + episode ends |

The C++ check catches cases where the Python snapshot is stale (e.g., a previous move in the same step changed connectivity).

---

## Reward Function

The total reward Python receives per step is:

```
R_total = R_sim + 0.1 · (Φ(s') − Φ(s))
```

**R_sim** — C++ per-step reward (`computeReward()` in `gnnShapeReconfigurationBlockCode.cpp`):

```
R_sim = (new_in_target − prev_in_target) − 0.01 + 10·done_success
```

| Term | Meaning |
|---|---|
| `new_in_target − prev_in_target` | Change in the number of blocks occupying target cells this step (positive when a block enters, negative when one leaves) |
| `−0.01` | Per-step cost — encourages reaching the goal quickly |
| `+10·done_success` | Bonus when all target cells are filled |
| `−10` (override) | Connectivity violation — episode ends immediately |

**Φ(s) — shape potential** (Python, `env.py: _potential()`):

```
Φ(s) = − mean over non-target blocks of L1-distance to the nearest target cell
```

The shaping term `0.1 · (Φ(s') − Φ(s))` is positive when blocks move closer to unfilled target cells. Φ is `0` when there are no non-target blocks (i.e., the shape is complete) or when no target is defined.

> **Contrast with locomotion**: the locomotion variant uses `R_sim = 0` (connectivity penalty only) and `Φ(s) = dot(CoM, direction)`.

---

## Action Masking

Before passing tensors to the policy, `obs_to_tensors()` builds a boolean mask `[N, 13]`:

| Block state | Mask |
|---|---|
| Articulation point | `[True, False×12]` — stay only |
| Already `in_target` | `[True, False×12]` — stay only (lock-in) |
| Normal block with `k` valid moves | `[True×(k+1), False×(12−k)]` |

The **in-target lock** is specific to shape reconfiguration: a block already placed in the target shape is forbidden from moving away. This stabilises training and prevents oscillation around the goal.

Masking is applied inside the policy (`logits.masked_fill(~mask, −∞)`) before softmax.

---

## Action Space

Per block, `N_ACTIONS = 13`:

| Index | Action |
|---|---|
| `0` | Stay in place |
| `1..k` | Move to `motions[i-1]` from `SlidingCubesBlock::getAllMotions()` |
| `k+1..12` | Masked out (no corresponding motion available) |

`getAllMotions()` enumerates the kinematically valid destinations under the sliding-cube motion rules — slides along edges and rotations over corners. The maximum supported is 12 (6 faces × 2 rotation types in dense conditions), but typical blocks see 2–6.

---

## GNN Policy (`model.py`)

### Architecture

```
Input: node features [N, 12], edge features [E, 6]
  │
  ├── Encoder: 3 × GNN layer (GAT or GCN) → [N, 64]
  │
  ├── Actor head:  Linear(64,64) → ELU → Linear(64,13) → logits [N, 13]   (per node)
  └── Critic head: global mean-pool → Linear(64,64) → ELU → Linear(64,1) → scalar
```

### Node features (12 dims)

| Index | Feature | Description |
|---|---|---|
| 0–2 | Normalised position | `pos / grid_size` per axis |
| 3 | **In-target flag** | `1.0` if the block occupies a target cell, else `0.0` |
| 4–9 | Neighbour presence | One bit per direction (PlusX, MinusX, PlusY, MinusY, PlusZ, MinusZ) |
| 10 | Normalised move count | `min(n_moves, 12) / 12` |
| 11 | Articulation-point flag | `1.0` if removing this block disconnects the cluster |

Feature x(3) (`in_target`) tells the policy which blocks are already placed and should not move, and which still need to be redirected. This is the key feature that differentiates shape reconfiguration from locomotion (where x(3) is `directionalPos` instead).

### Edge features (6 dims)

One-hot encoding of the connector direction (PlusX … MinusZ).

### Backbone options

**GAT** (`backbone='gat'`, default): Graph Attention Network. Each layer computes attention weights over neighbours. Richer representation; recommended for training.

**GCN** (`backbone='gcn'`): `GCNEdgeConv` — mean aggregation with edge features concatenated into the message:

```
msg_{j→i}  = W_msg · [h_j ‖ e_{ji}]
h_i'       = mean_{j∈N(i)}(msg_{j→i}) + W_self · h_i + b
```

Lower parameter count; the hop-count / information-radius relationship is explicit. Required for on-robot deployment.

### Distributed inference property

After `L = 3` message-passing layers, each node's hidden state depends only on its `L`-hop neighbourhood. `distributed_act()` runs the encoder + actor only (no global pooling), so each physical module can execute it independently after `L` rounds of local message exchange with neighbours.

---

## PPO Training (`ppo.py`)

| Hyperparameter | Value |
|---|---|
| Learning rate | 3 × 10⁻⁴ |
| Rollout length (`n_steps`) | 256 steps before each update |
| Epochs per update | 4 |
| Mini-batch size | 32 transitions |
| Clip ε | 0.2 |
| Value coefficient | 0.5 |
| Entropy coefficient | 0.01 |
| Max grad norm | 0.5 |
| GAE λ | 0.95 |
| Discount γ | 0.99 |
| Optimiser | Adam |

Advantages are computed with **Generalized Advantage Estimation (GAE)** and normalised globally across the rollout. The value function is a single global scalar (centralised critic); advantages are broadcast to all nodes in the graph. The policy gradient uses the **clipped surrogate objective** (standard PPO). Per-node log-probabilities are averaged over all nodes for the policy loss.

### Early stopping

Training stops early when the rolling solve rate over the last `SUCCESS_WINDOW = 200` episodes is at least `SUCCESS_THRESH = 0.95`, provided at least `SUCCESS_MIN_EP = 200` episodes have completed. An episode counts as solved when every target cell is occupied by some block before `MAX_STEPS`.

---

## Episode Configuration (`config.xml`)

The default configuration places 6 modules and a 6-cell target shape on an 8×6×4 grid:

- **Initial shape**: blocks at `(1..3, 1..2, 0)` — 6 modules forming a 2×3 rectangle.
- **Target**: 6 cells at `(3..4, 1, 0..2)` — a 2×3 rectangle rotated into the X-Z plane and shifted. Blocks at `(3,1,0)` start already in the target (1/6 overlap).
- **Max steps**: 200 per episode.

`MAX_STEPS` and `GYM_PORT` are constants in `gnnShapeReconfigurationBlockCode.hpp` and require a rebuild to change.

---

## Distributed Deployment (deploy mode)

Deploy mode runs the trained GCN policy directly inside the simulator with **no Python loop and no TCP**. Each module loads the same weights file at startup and runs its own copy of the GCN forward pass in C++ (Eigen). Modules exchange neighbourhood information over P2P messages, independently compute their preferred action, then elect a single winner via a shared in-process ballot.

### Components

- **`train/export_weights.py`** — exports the trained `GNNPolicy(backbone='gcn')` weights to a flat little-endian binary (CRC-32 verified). Header: magic `GCN1`, version, `hidden=64`, `node_dim=12`, `edge_dim=6`, `n_actions=13`, `n_layers=3`. Body: 3 layers × (`msg_lin.weight`, `self_lin.weight`, `bias`), then actor `fc1`/`fc2` weights and biases, then a CRC-32 trailer.
- **`gcnPolicy.hpp` / `.cpp`** — pure C++ Eigen port of the GCN forward pass. Singleton `GCNWeights::instance()` loads once; all modules share the read-only copy. `buildNodeFeature` accepts `bool inTarget` to populate x(3).
- **`gnnShapeReconfigurationBlockCode` (deploy path)** — activated by `<deploy>` element or `GNN_DEPLOY_WEIGHTS` env var. The `<targetList>` from the same XML is used to compute each module's `in_target` flag during node feature construction.

### Per-step protocol

Each step executes a fixed sequence of P2P rounds scheduled via `InterruptionEvent` and `t0 + ROUND_DT_US` deltas (default `2000 µs`). Every payload carries `step` so stale messages from prior steps are dropped.

```
DEPLOY_STEP_START  (interruption, fired simultaneously for all modules)
  ├─ Phase A round 1   MSG_TOPO_1HOP    broadcast 1-hop neighbour mask + id
  ├─ Phase A round 2   MSG_TOPO_2HOP    broadcast neighbour ids → each node has 2-hop subgraph
  │                   ↳ compute is_ap locally via reachability over 2-hop induced subgraph
  │                   ↳ read inTarget from local target predicate
  │                   ↳ build 12-d node feature h⁰
  ├─ Phase B round 1   MSG_GNN_LAYER1   send msg_lin([h⁰ ‖ e_ji]) to each neighbour → h¹
  ├─ Phase B round 2   MSG_GNN_LAYER2   → h²
  ├─ Phase B round 3   MSG_GNN_LAYER3   → h³
  └─ Phase C           actorLogits(h³) → anti-oscillation check → register vote in ballot
                       last voter picks winner → one moveTo() → schedule next steps
```

A round advances when all connected interfaces have delivered their message (common case) or a watchdog fires. Modules whose neighbour is mid-motion tolerate missing messages: an isolated node treats the aggregated term as zero.

#### Watchdog safety

Each phase has a dedicated watchdog `InterruptionEvent` that fires `3 × ROUND_DT_US` after the phase begins. Watchdogs carry prerequisite guards to prevent stale events from a previous step corrupting the current step's state:

| Watchdog              | Guard before firing              |
|-----------------------|----------------------------------|
| `DEPLOY_TIMEOUT_TOPO1`  | `!topo2Sent`                   |
| `DEPLOY_TIMEOUT_TOPO2`  | `!topo2Advanced`               |
| `DEPLOY_TIMEOUT_LAYER1` | `!layerAdvanced[0] && topo2Advanced` |
| `DEPLOY_TIMEOUT_LAYER2` | `!layerAdvanced[1] && layerAdvanced[0]` |
| `DEPLOY_TIMEOUT_LAYER3` | `!layerAdvanced[2] && layerAdvanced[1]` |

### Message payloads

| Type id | Name              | Payload struct                                            |
|---------|-------------------|-----------------------------------------------------------|
| 100     | `MSG_TOPO_1HOP`   | `{ step, id, mask6 }`                                     |
| 101     | `MSG_TOPO_2HOP`   | `{ step, id, mask6, nbIds[6] }`                           |
| 200     | `MSG_GNN_LAYER1`  | `{ step, fromDir, h[64] }` (msg_lin([h⁰ ‖ e_ji]))         |
| 201     | `MSG_GNN_LAYER2`  | `{ step, fromDir, h[64] }`                                |
| 202     | `MSG_GNN_LAYER3`  | `{ step, fromDir, h[64] }`                                |

`fromDir` is the direction the *receiver* sees the sender on (i.e., `d ^ 1`; SCLattice2 directions are paired Plus/Minus per axis).

### Local articulation-point detection

Each module's 2-hop induced subgraph (built from `MSG_TOPO_2HOP` messages) is enough to test whether removing this module disconnects its direct neighbours from one another:

1. Build adjacency over `{ self ∪ direct neighbours ∪ 2-hop ids }` from the topology messages.
2. Pick any direct neighbour as BFS source; run BFS *excluding* self.
3. If any direct neighbour is unreachable from the source, declare self an articulation point.

### Action selection and winner ballot

After the GNN forward pass each module applies the shape-reconfiguration action mask:

```
mask[0] = true                                  // stay always allowed
if !inTarget && !isAP:
    mask[1..nMoves] = true                      // move actions available
```

i.e., **both** the articulation-point lock **and** the in-target lock are enforced — the same two constraints used during training. Selection uses `argmaxMasked` for deterministic deployment.

**Anti-oscillation filter**: before registering the vote, each module checks its proposed destination against a circular history of the last `HIST_LEN = 4` positions it occupied. If the destination matches any recent position, the action is suppressed to stay. If every module votes stay because of history, all histories are cleared (deadlock escape).

**Shared ballot** (`static DeployBallot`): every module registers `{blockId, action, self*, motions}`. When all N modules have voted:

1. The last voter picks the **winner**: the lowest block ID with a non-stay action.
2. The winner's `moveTo()` is the only `moveTo()` issued per step.
3. The winner's current position is pushed into its position history.
4. If shape complete or step budget exceeded, no next step is scheduled.
5. Otherwise the last voter schedules `DEPLOY_STEP_START` for **all** modules at the same future simulation time.

**Termination**: episode ends on shape completion (`countBlocksInTarget() >= |target|`) or step-budget exhaustion (`deployStep >= MAX_STEPS`).

### Step synchronisation and motion pacing

```
nextStep = now + max(motionDuration, 8 × ROUND_DT_US)
```

where `motionDuration = 1 100 000 µs` when a move was issued, else `0`.

### Configuration

```xml
<world ...>
  <deploy enabled="true" weights="gcn_weights.bin" seed="42"/>
  <blockList ...> ... </blockList>
  <targetList> ... </targetList>
</world>
```

`weights` is resolved relative to the simulator's working directory (`applicationsBin/gnnShapeReconfiguration/`). The env var `GNN_DEPLOY_WEIGHTS=<absolute_path>` overrides the XML setting.

---

## Comparison with gnnLocomotion

| Aspect | gnnShapeReconfiguration | gnnLocomotion |
|---|---|---|
| **Objective** | Fill a target shape | Translate CoM along a direction |
| **Node feature x(3)** | `in_target` (bool) | `directionalPos` = `dot(pos,dir)/extent` |
| **Action mask** | AP + in-target lock | AP only |
| **Reward (C++)** | `Δin_target − 0.01 + 10·done` | `0` (connectivity penalty only) |
| **Reward (Python)** | `R_sim + 0.1 × ΔΦ_shape` | `R_sim + ΔΦ_locomotion` |
| **Potential Φ** | `−mean L1 dist to nearest target` | `dot(CoM, direction)` |
| **Episode done** | All blocks in target OR max steps | Max steps only |
| **Config** | `<targetList>` | `<locomotion direction="..."/>` |
| **Early stop** | Solve-rate ≥ 0.95 over 200 episodes | None |
| **Deploy log** | `inTarget=0/1` | `dp=<float>` |

### Known limitations

- **Curriculum required for harder targets**: the default config has a 1/6 overlap between initial and target. Larger displacement targets typically require a curriculum (gradually increasing target offset) to converge.
- **2-hop AP detection** is exact only when every articulation point has a witness within 2 hops. Acceptable for clusters up to ~20 modules in dense lattices.
- **One mover per step**: the ballot currently selects one mover per step. Extending to top-k movers would speed up reconfiguration.
- **In-target lock during training**: blocks already in the target cannot move, which prevents some otherwise efficient solutions where a placed block briefly swaps out to let another pass. Removing the lock destabilises training; the trade-off is intentional.
