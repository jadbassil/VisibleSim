# GNN Locomotion — Architecture

## Overview

This application trains a Graph Neural Network (GNN) policy to control a cluster of modular sliding-cube robots to **locomote as a whole in a given direction**. Each module independently decides whether to move and where, guided by a shared reward signal: the displacement of the swarm's center of mass (CoM) along the target direction.

The C++ simulator (VisibleSim) acts as the physics engine and is controlled by a Python training loop over a TCP socket. The same GCN weights can then be deployed directly inside the simulator with no Python process.

> **Relationship to `gnnShapeReconfiguration`**: this application shares the same distributed GNN infrastructure (gym protocol, deploy protocol, PPO loop) but differs in reward definition, node feature x(3), action mask, and termination condition. See the summary table at the bottom of this section.

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
  "grid_size": [12, 6, 4],
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
  "target": []
}
```

- `reward` — C++ sends `0.0` for normal steps; connectivity-violation penalty (`−10`) is the only non-zero value. The main locomotion reward is computed entirely by Python's potential shaping.
- `in_target` — **repurposed**: `true` if `directionalPos > 0.5`, i.e., this block is in the leading half of the swarm along the travel direction. The field name is kept for protocol compatibility; it no longer indicates goal placement.
- `target` — always an empty array; locomotion has no goal shape.
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

The total step reward received by Python is:

```
R_total = R_sim + Φ(s') − Φ(s)
```

**Φ(s) — locomotion potential** (Python, `env.py: _potential()`):

```
Φ(s) = dot(CoM(s), direction)
```

where `CoM(s)` is the mean position of all blocks and `direction` is the normalised travel vector (e.g., `[1, 0, 0]` for +X). The shaping reward `F = Φ(s') − Φ(s)` equals the CoM displacement along the travel axis per step — positive whenever the swarm moves forward.

**R_sim** (from C++):
- `0.0` on normal steps (locomotion has no per-block goal signal)
- `−10` connectivity-violation penalty (episode ends immediately)

> **Contrast with shape reconfiguration**: shape reconfiguration used `R_sim = (new_in_target − prev_in_target) − 0.01 + 10 × done` and `Φ(s) = −mean L1 distance to nearest target cell`. The locomotion objective replaces both with a single CoM-progress signal.

---

## Action Masking

Before passing tensors to the policy, `obs_to_tensors()` builds a boolean mask `[N, 13]`:

| Condition | Mask |
|---|---|
| Block is an articulation point | `[True, False×12]` — stay only |
| Normal block with `k` valid moves | `[True×(k+1), False×(12−k)]` |

There is **no in-target lock** for locomotion — every non-AP block is free to move at every step. This is a key difference from shape reconfiguration, where blocks already placed in the target were forced to stay.

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

| Index | Feature | Description |
|---|---|---|
| 0–2 | Normalised position | `pos / grid_size` per axis |
| 3 | **Directional position** | `dot(pos, direction) / grid_extent` — how far this block is along the travel axis (replaces `in_target` from shape reconfiguration) |
| 4–9 | Neighbour presence | One bit per direction (PlusX … MinusZ) |
| 10 | Normalised move count | `n_moves / 12` |
| 11 | Articulation-point flag | `1.0` if removing this block disconnects the cluster |

Feature x(3) tells the policy which blocks are at the leading vs. trailing edge of the swarm. Leading blocks (high `directionalPos`) benefit from moving further forward; trailing blocks (low `directionalPos`) may need to detach and leap-frog. The `in_target` concept is not used here.

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

---

## PPO Training (`ppo.py`)

| Hyperparameter | Value |
|---|---|
| Learning rate | 3 × 10⁻⁴ |
| Rollout length | 256 steps (before each update) |
| Epochs per update | 4 |
| Mini-batch size | 32 transitions |
| Clip ε | 0.2 |
| Value coefficient | 0.5 |
| Entropy coefficient | 0.01 |
| GAE λ | 0.95 |
| Discount γ | 0.99 |

Advantages are computed with **Generalized Advantage Estimation (GAE)** and normalised globally across the rollout. The value function is a single global scalar (centralised critic); advantages are broadcast to all nodes in the graph. The policy gradient uses the **clipped surrogate objective** (standard PPO). Per-node log-probabilities are averaged over all nodes for the policy loss.

There is no early-stopping solve-rate criterion because locomotion has no binary success condition. Training runs for `--episodes` iterations or until interrupted.

---

## Episode Configuration (`config.xml`)

The default configuration places 6 modules in a 2×3 rectangle on a 12×6×4 grid:

- **Initial shape**: blocks at `(1..3, 1..2, 0)` — 6 modules
- **Travel direction**: `[1, 0, 0]` (+X axis)
- **No target shape** — reward is accumulated CoM displacement in +X
- **Max steps**: 200 per episode

The `<locomotion direction="dx,dy,dz"/>` XML element sets the travel direction for both the C++ feature computation and (via the `--direction` argument) the Python reward. The direction is normalised at runtime.

---

## Distributed Deployment (deploy mode)

Deploy mode runs the trained GCN policy directly inside the simulator with **no Python loop and no TCP**. Each module loads the same weights file at startup and runs its own copy of the GCN forward pass in C++ (Eigen). Modules exchange neighbourhood information over P2P messages, independently compute their preferred action, then elect a single winner via a shared in-process ballot.

### Components

- **`train/export_weights.py`** — exports the trained `GNNPolicy(backbone='gcn')` weights to a flat little-endian binary (CRC-32 verified). Header: magic `GCN1`, version, `hidden=64`, `node_dim=12`, `edge_dim=6`, `n_actions=13`, `n_layers=3`. Body: 3 layers × (`msg_lin.weight`, `self_lin.weight`, `bias`), then actor `fc1`/`fc2` weights and biases, then a CRC-32 trailer.
- **`gcnPolicy.hpp` / `.cpp`** — pure C++ Eigen port of the GCN forward pass. Singleton `GCNWeights::instance()` loads once; all modules share the read-only copy. Key difference from shape reconfiguration: `buildNodeFeature` accepts `float directionalPos` instead of `bool inTarget`.
- **`gnnLocomotionBlockCode` (deploy path)** — activated by `<deploy>` element or `GNN_DEPLOY_WEIGHTS` env var. The `<locomotion direction="..."/>` element in the same XML is parsed at startup to set `locomotionDir[3]`, which is used to compute each module's `directionalPos` during node feature construction.

### Per-step protocol

Each step executes a fixed sequence of P2P rounds scheduled via `InterruptionEvent` and `t0 + ROUND_DT_US` deltas (default `2000 µs`). Every payload carries `step` so stale messages from prior steps are dropped.

```
DEPLOY_STEP_START  (interruption, fired simultaneously for all modules)
  ├─ Phase A round 1   MSG_TOPO_1HOP    broadcast 1-hop neighbour mask + id
  ├─ Phase A round 2   MSG_TOPO_2HOP    broadcast neighbour ids → each node has 2-hop subgraph
  │                   ↳ compute is_ap locally via reachability over 2-hop induced subgraph
  │                   ↳ compute directionalPos = dot(pos, locomotionDir) / extent
  │                   ↳ build 12-d node feature h⁰
  ├─ Phase B round 1   MSG_GNN_LAYER1   send msg_lin([h⁰ ‖ e_ji]) to each neighbour → h¹
  ├─ Phase B round 2   MSG_GNN_LAYER2   → h²
  ├─ Phase B round 3   MSG_GNN_LAYER3   → h³
  └─ Phase C           actorLogits(h³) → anti-oscillation check → register vote in ballot
                       last voter picks winner → one moveTo() → schedule all next steps
```

A round advances when all connected interfaces have delivered their message (common case) or a watchdog fires. Modules whose neighbour is mid-motion tolerate missing messages: an isolated node treats the aggregated term as zero.

#### Watchdog safety

Each phase has a dedicated watchdog `InterruptionEvent` that fires 3 × `ROUND_DT_US` after the phase begins. Watchdogs carry prerequisite guards to prevent stale events from a previous step corrupting the current step's state:

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

After the GNN forward pass each module applies the locomotion action mask (`is_ap` → force stay; all other blocks can move) and runs `argmaxMasked`. There is no `in_target` lock.

**Anti-oscillation filter**: before registering the vote, each module checks its proposed destination against a circular history of the last `HIST_LEN = 4` positions it occupied. If the destination matches any recent position, the action is suppressed to stay.

**Shared ballot** (`static DeployBallot`): every module registers `{blockId, action, self*, motions}`. When all N modules have voted:
1. The last voter picks the **winner**: the lowest block ID with a non-stay action.
2. The winner's `moveTo()` is the only `moveTo()` issued per step.
3. The winner's current position is pushed into its position history.
4. **Deadlock escape**: if all blocks voted stay (histories blocked every option), all histories are cleared.
5. The last voter schedules `DEPLOY_STEP_START` for **all** modules at the same future simulation time.

**Termination**: only on step-budget exhaustion (`deployStep >= MAX_STEPS`). There is no shape-completion check.

### Step synchronisation and motion pacing

```
nextStep = now + max(motionDuration, 8 × ROUND_DT_US)
```

where `motionDuration = 1 100 000 µs` when a move was issued.

### Configuration

```xml
<world ...>
  <deploy enabled="true" weights="gcn_weights.bin" seed="42"/>
  <locomotion direction="1,0,0"/>
  <blockList ...> ... </blockList>
  <!-- no targetList -->
</world>
```

The `<locomotion direction="dx,dy,dz"/>` element must be present in both training (`config.xml`) and deploy (`config_deploy.xml`) configs and **must match the `--direction` used during training** so that the directional position feature is consistent. The direction is normalised to a unit vector at runtime.

`weights` is resolved relative to the simulator's working directory (`applicationsBin/gnnLocomotion/`). The env var `GNN_DEPLOY_WEIGHTS=<absolute_path>` overrides the XML setting.

---

## Comparison with gnnShapeReconfiguration

| Aspect | gnnShapeReconfiguration | gnnLocomotion |
|---|---|---|
| **Objective** | Fill a target shape | Translate CoM along a direction |
| **Node feature x(3)** | `in_target` (bool) | `directionalPos` = `dot(pos,dir)/extent` |
| **Action mask** | AP + in-target lock | AP only |
| **Reward (C++)** | `Δin_target − 0.01 + 10×done` | `0` (connectivity penalty only) |
| **Reward (Python)** | `R_sim + 0.1 × ΔΦ_shape` | `R_sim + ΔΦ_locomotion` |
| **Potential Φ** | `−mean L1 dist to nearest target` | `dot(CoM, direction)` |
| **Episode done** | All blocks in target OR max steps | Max steps only |
| **Config** | `<targetList>` | `<locomotion direction="..."/>` |
| **Train arg** | (none for direction) | `--direction dx,dy,dz` |
| **Deploy log** | `inTarget=0` | `dp=0.25` |

### Known limitations

- **Policy convergence**: the GCN was trained on the locomotion objective but the directional-position feature changes over time as the swarm moves. Later in an episode the feature distribution drifts from the initial training distribution (blocks start near `x=1..3` but may be near `x=8+` after 100 steps). Curriculum learning or domain randomisation over starting positions can mitigate this.
- **2-hop AP detection** is exact only when every articulation point has a witness within 2 hops. Acceptable for clusters up to ~20 modules in dense lattices.
- **One mover per step**: the ballot currently selects one mover per step. Extending to top-k movers would increase locomotion speed.
