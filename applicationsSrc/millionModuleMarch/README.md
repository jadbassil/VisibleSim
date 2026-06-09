# Million Module March — Catoms3D locomotion

A VisibleSim application that implements the locomotion algorithm of

> R. Fitch and Z. Butler, *"Million Module March: Scalable Locomotion for Large
> Self-Reconfiguring Robots"*, **IJRR** 27(3–4):331–343, 2008.

adapted to 3D catoms (`Catoms3D`). A lattice robot marches across the
environment, over obstacles, toward a sequence of overlapping goal bounding
boxes — when the current box is filled the goal advances and the robot
re-forms beyond it.

## Algorithm (paper → this app)

| Paper (SlidingCube MDP)              | This app (Catoms3D)                                                |
|--------------------------------------|--------------------------------------------------------------------|
| State = lattice position             | FCC lattice cell (`Cell3DPosition`)                                 |
| Action = slide / convex transition   | One pivot **rotation**; legal set = `Catoms3DMotionEngine::getAllReachablePositions` |
| Reward −1/step, 0 in goal (− height) | BFS distance-to-goal field; ties broken toward the goal, then low z |
| DP value function (Bellman backups)  | Converged value function = multi-source BFS from the goal cells     |
| Connecting-cycle mobility check      | Bounded local connecting-cycle search before every move            |
| Test-and-set lock, lower-ID deadlock | Atomic destination-cell + pivot reservation                        |
| Moving goal bounding box             | `Target` + `Target::loadNextTarget()` once a box is filled         |

The **3D-catom motion constraints** are enforced for free: a module only ever
considers cells returned by `getAllReachablePositions`, i.e. positions reachable
by a single, collision-free, connector-valid pivot rotation.

## Build (CMake)

From the repository root:

```bash
cmake -B build .
cmake --build build --target millionModuleMarch
```

The binary is written to `applicationsBin/millionModuleMarch/millionModuleMarch`.
(A `Makefile` is also provided for parity with the other `applicationsSrc/*`
apps: `make -C applicationsSrc/millionModuleMarch`.)

## Run

```bash
cd applicationsBin/millionModuleMarch

# GUI:
./millionModuleMarch -c config.xml

# Headless (note: the periodic planner means the event queue never empties,
# so cap the run with -s <microseconds>):
./millionModuleMarch -t -s 80000000 -c config.xml
```

Module colours: grey = obstacle, white = rotating, red = reserved as a pivot,
green = placed in the goal, otherwise shaded by distance-to-goal.

### Configurations

- **`config.xml`** — small demo: a 2×2×2 cube (8 catoms) marches over a one-tall
  obstacle, fills goal box 1, advances the goal, crosses the obstacle and fills
  goal box 2. Completes end-to-end (`ALL GOAL BOXES REACHED`).
- **`config_cube.xml`** — larger demo: a 3×3×3 cube (27 catoms) marching over a
  two-tall obstacle. The whole robot shifts forward and reaches each box.
- **`config_march.xml`** — *self-generated* marching goal (see below): a 2×2×2
  cube walks several body-lengths over flat ground / a small obstacle with no
  hand-authored boxes.

Obstacles are `<block ... obstacle="true"/>`; explicit goal boxes are successive
`<target format="grid">` elements (do **not** put XML comments directly inside
`<targetList>` — the core parser does not skip them).

### Goal modes

The goal can be specified two ways:

1. **Explicit boxes** (`config.xml`, `config_cube.xml`): a `<targetList>` of
   successive `<target>` regions. When one is filled the goal advances to the
   next.

2. **Self-generated marching goal** (`config_march.xml`): *omit* `<targetList>`.
   The goal becomes the robot's own bounding box used as a window that slides
   forward along a chosen axis; the robot continuously re-forms its shape one
   step ahead. This is the paper's flat-ground experiment — *"move forward N
   module widths and reform the cube"* (Section 5.2) — and needs no authored
   boxes. It is driven by `<world>` attributes:

   | attribute     | meaning                                            | default |
   |---------------|----------------------------------------------------|---------|
   | `marchAxis`   | travel axis: `x`/`y` (horizontal), `z` (vertical)  | `x`     |
   | `marchDir`    | `1` (toward +axis) or `-1`                         | `1`     |
   | `marchSteps`  | number of forward steps to walk                    | `3`     |
   | `marchStride` | cells the window advances per step (overlap)       | `1`     |

   Example: `<world gridSize="22,8,7" marchAxis="x" marchSteps="10">`.
   Total distance travelled ≈ `marchSteps × marchStride` cells.

In march mode the goal advances when the robot's trailing edge has translated
into the window (a compact step), or — if the tail momentarily sticks while the
body still overlaps the window — the higher-level process **nudges** the window
forward to pull it loose (the paper's joystick). A small stride keeps each goal
heavily overlapping the body so the robot never strands or runs away.

In explicit-box mode the goal advances when its box is **filled** or the robot
has **settled** (handles the paper's last-module contention). Each advance is
logged as `(reached)` / `(nudged)` (march) or `(filled)` / `(settled)` (boxes).

## Implementation note (realism)

VisibleSim is a single-process discrete-event simulator. The paper's planning is
a *distributed* dynamic program (neighbour-to-neighbour Bellman backups) and its
actuation uses *distributed* test-and-set locking with lower-ID deadlock
resolution. Implemented literally with asynchronous messages, both misbehave on a
constantly-changing structure: the value field oscillates (Bellman-Ford
count-to-infinity) and pivot locks leak when a mover ends a rotation no longer
adjacent to its pivot (livelock).

This implementation therefore realizes the **converged outcome** of those
distributed protocols directly as shared state — the value function is the
multi-source BFS the backups converge to, and the locks are atomic claims (the
single-threaded event loop makes "test, then claim" atomic, which is exactly the
mutual exclusion the locking round provides). The algorithm's *logic* — MDP value
function, greedy policy descent, connecting-cycle connectivity preservation,
destination/pivot locking, obstacle handling and a moving goal — is unchanged.
The local connecting-cycle mobility test is computed over the local neighbourhood
as the paper specifies.
