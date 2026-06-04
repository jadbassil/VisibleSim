# metaModuleGen — automatic meta-module generator for 3D Catoms

A standalone VisibleSim application that searches for the **smallest** valid
meta-module satisfying the formal framework of Lamiroy, Bassil & Piranda
(*A Formal Framework for Meta-Module Design in Modular Robot-Based
Programmable Matter*). A meta-module is valid only if it admits all three
relocation operations — **Disassembly**, **Transfer** (the flow to a
neighbor) and **Assembly** — in **each of the six ±-directions**, and the
search minimizes the total motion cost of those operations. The search
runs a genetic algorithm; the underlying motion predicate is implemented
for the FCC lattice used by 3D Catoms.

The generator is fully decoupled from the live VisibleSim runtime so a
single fitness evaluation does not require booting the GLUT viewer or
instantiating `Catoms3DBlock` objects. Once the GA finishes, the best
shape is exported to a config XML and (optionally) handed to the standard
Catoms3D viewer for inspection.

## Files

| File | Role |
|---|---|
| [metaModuleCore.h](metaModuleCore.h) / [metaModuleCore.cpp](metaModuleCore.cpp) | Standalone FCC neighbor + `canMove` predicate, MetaModule type, geometric validators, bridge router, Transfer / Disassembly / Assembly planners, evaluator, GA |
| [metaModuleGen.cpp](metaModuleGen.cpp) | `main()`: CLI parsing, GA driver, XML export, optional viewer boot |
| [metaModuleGenBlockCode.h](metaModuleGenBlockCode.h) / [.cpp](metaModuleGenBlockCode.cpp) | Interactive `Catoms3DBlockCode`: parses `<metaModuleInfo>`, lets you click-select a meta-module and animate Dismantle / Transfer / Assemble in any direction |
| [Makefile](Makefile) / `CMakeLists.txt` target | Two interchangeable build paths |

## Building

The application is wired into both build systems. Pick whichever you
prefer.

**CMake (recommended for IDEs):**
```sh
cmake -B build .
cmake --build build --target metaModuleGen
```

**Make:**
```sh
make -C applicationsSrc/metaModuleGen
```

The binary is dropped into [applicationsBin/metaModuleGen/](../../applicationsBin/metaModuleGen/).

## Running

```sh
cd applicationsBin/metaModuleGen

# headless search + XML export + GLUT viewer on the discovered shape
./metaModuleGen --min-size=4 --max-size=20 --radius=3 --pop=80 --gens=100

# headless only (no viewer) — useful for batch experiments
./metaModuleGen --no-sim --min-size=4 --max-size=20 --pop=120 --gens=200

# quick replay of a cached result
./metaModuleGen --min-size=7 --max-size=7 --pop=4 --gens=1
```

### CLI flags

| Flag | Default | Meaning |
|---|---|---|
| `--no-sim` | off | Skip the GLUT viewer; only print + export the result |
| `--view=PATH` | — | Skip the search entirely and open the interactive viewer on an existing structure XML (must contain `<metaModuleInfo>`). Lets you revisit a result without re-running the GA. |
| `--min-size=N` | 4 | Smallest `\|M\|` tested by the outer size loop |
| `--max-size=N` | 30 | Upper bound on `\|M\|` before giving up |
| `--radius=N` | 4 | Half-extent of the bounding box `[-N..N]³` cells in which the shape lives |
| `--pop=N` | 120 | GA population per `(basis, size)` |
| `--gens=N` | 200 | GA generations per `(basis, size)` |
| `--seed=N` | 42 | RNG seed |
| `--tiles=N` | 2 | Export an `N×N×N` block of meta-modules in the structure XML (the visualized tiling) |
| `--out-xml=PATH` | `metaModule_result.xml` | Where to write the exported VisibleSim config |

> **Build with optimization for usable GA speed.** Evaluating all 18
> `(operation, direction)` pairs with the bridge DFS is BFS-heavy; an
> unoptimized (`-g`/`-O0`) build makes the search crawl. Build the app with
> `-O2` (e.g. pass `GLOBAL_CCFLAGS="-O2 -std=c++17 ..."`), or just use
> `--view=metaModule_result.xml` to open a previously-found structure
> instantly.

Any other flag (`-r`, `-R`, `-c file.xml`, `-t`, `-s`, …) is forwarded
unchanged to `createSimulator`. The application auto-injects `-r` only if
no scheduler flag (`-r`/`-R`/`-t`) is present, so the viewer launches by
default without overriding a terminal-mode (`-t`) run.

### Output

On success the binary prints the chosen basis, the cells of the
meta-module, the proposed `center`, and a per-direction cost breakdown
for all three operations:

```
*** Best meta-module ***
basis label : ortho-3-3-2
MetaModule { size=6 basis=( X=(3,0,0), Y=(0,3,0), Z=(0,0,2))
             center=(-2,-1,-1) cells={...} }
Transfer cost per direction:
    +X  feasible=yes  bridges=0  setup=0  perModule=3  clear=0  total=3
    ...
Disassembly / Assembly moves per direction:
    +X  disasm feasible=yes moves=14   asm feasible=yes moves=17
    ...
totals (sum over ±X, ±Y, ±Z):  transfer=30  disassembly=98  assembly=91  ALL=219
```

It also writes a VisibleSim-style config XML (default
`metaModule_result.xml`) that the viewer can ingest directly.

## Visualizing & interacting with the operations

Unless `--no-sim` is given, the generator boots the Catoms3D viewer on the
exported structure. The config tiles the discovered meta-module into a
`2×2×2` block of meta-cells (distinct colors per cell; configurable with
`--tiles=N`) and embeds a
`<metaModuleInfo>` element describing the lattice, so the viewer is
interactive:

| Input | Action |
|---|---|
| **click** a module, then press `o`/`k` | select its meta-module (highlighted red) |
| `o` | cycle the operation: Dismantle → Transfer → Assemble |
| `k` | cycle the direction: `+X −X +Y −Y +Z −Z` |
| `e` | execute the selected operation in the chosen direction |
| `v` | toggle motion style: real Catoms3D rotations ⇄ teleport |
| `c` | reprint the controls |

(`g`/`h` are reserved by the viewer for shadows, so execute is `e` and help is `c`.)

The operation is animated **one motion at a time** via a reactive queue: each
hop is played as a real `Catoms3DRotationStartEvent` (rolling animation) and
only the next hop is scheduled once the previous `onMotionEnd` fires — so each
rotation is computed from the module's up-to-date position. If the engine can't
find a valid rotation for a hop, that hop falls back to a teleport (`v` forces
teleport for all hops).

The chosen operation is animated by teleporting modules cell-by-cell:

- **Dismantle(`V`)** relocates the selected meta-module into the empty
  neighbor slot along `V` (emptying the source), in disassembly order.
- **Assemble(`V`)** builds the neighbor slot along `V`, in build order.
- **Transfer(`V`)** flows modules from the `−V` neighbor (if full),
  through the selected meta-module, into the empty `+V` slot.

The target slot must be empty and the source full, otherwise the operation
prints why it was skipped. State + scheduling messages print to the
terminal. (Teleportation is a placeholder motion, not physically-accurate
catom rotation — see Known limitations.)

## Algorithm

The generator follows the framework of the paper section by section.
Below is the pipeline; cross-references in **bold** point to specific
items in the paper.

### 1. Lightweight FCC motion predicate

To run millions of fitness evaluations, the application uses its own
`canMove(s, t, occupied)` predicate operating purely on
`Cell3DPosition` sets, rather than driving the full
`Catoms3DMotionEngine`. The predicate mirrors the geometry of
[`isNotLockedForMotion`](../../simulatorCore/src/robots/catoms3D/catoms3DMotionEngine.cpp):

- `t` must be free (not in `occupied`).
- Some pivot `p ∈ occupied` must be FCC-adjacent to both `s` and `t`.
- `t` must not be locked between a pair of opposing occupied cells
  (the OctaFace blocking rule; checked against the 6 antipodal
  neighbor pairs of `t`).

FCC neighborhoods come straight from `nCellsEven` / `nCellsOdd` in
[`lattice.h`](../../simulatorCore/src/grid/lattice.h) so a z-parity
flip yields the right 12-neighborhood automatically.

`reachableFrom(s, occ)` and `pathLength(s, t, occ)` BFS over the
predicate. Both are bounded by 400 visited cells so that pathological
shapes never freeze the GA.

### 2. Meta-module data model (paper Def. 4–6)

A candidate is `(M, basis, center)` where:

- `M ⊂ ℤ³` — finite, the set of occupied cells.
- `basis = (X⃗, Y⃗, Z⃗)` — the three translation vectors that tile
  meta-modules across space (paper Def. 5).
- `center ∈ ℤ³` — the rendezvous point used by Transfer (paper Def. 6).

### 3. Geometric validators

Before any expensive operation planning, each candidate must pass:

- **Connectivity** (Property 1, Eq. 6) — BFS over `M` using the FCC
  neighborhood must touch every cell.
- **Linear independence** of the basis (Property 2, Eq. 7) — `det ≠ 0`.
- **Exclusivity** (Property 2, Eq. 8) — no nonzero lattice shift
  `aX⃗ + bY⃗ + cZ⃗` maps any cell of `M` back into `M`. Implementation
  iterates `(a,b,c) ∈ {-1,0,1}³ \ {(0,0,0)}`, which is sufficient for
  the basis sizes considered.
- **Axis contacts** (Property 2, Eq. 9) — for each of `X⃗`, `Y⃗`, `Z⃗`
  there must exist a pair `(a, b) ∈ M²` with `a` FCC-adjacent to
  `b + V⃗`.
- **Center existence** (Def. 6, Eq. 10) — there must be a free cell
  adjacent to `M` that is not occupied by any lattice image of `M`.
  The first such cell found is used.

Failing any of these scores the candidate as infeasible immediately
(no Transfer planning attempted).

### 4. Transfer planner (paper §5.2)

For each `V⃗ ∈ {±X⃗, ±Y⃗, ±Z⃗}`:

1. Build the pivot set `C_piv = M ∪ (M + V⃗)` and the worst-case
   blocking set `C_blo` = union of all images of `M` under the lattice
   minus the source meta-module itself (treating every neighbor
   meta-module as fully present).
2. **Direct transfer**: try a BFS from `center` to `center + V⃗`
   inside `C_piv ∪ C_blo`. If reachable, the per-module transfer cost
   is the path length, with 0 bridges.
3. **With bridges**: when direct transfer fails, enumerate bridge
   candidates via a cheap heuristic (free cells touching ≥ 2 cells of
   `C_piv`) — this is the approximation of paper Algorithm 1. Then a
   DFS over subsets of size ≤ 2 of the closest 12 candidates places
   bridges incrementally and tests reachability. Setup cost is the
   sum of shortest paths from `center` to each bridge in order; clear
   cost is the sum of shortest paths from each bridge back to
   `center + V⃗` in reverse order.

A direction is infeasible if neither variant produces a path.

The single-module router (`routeWithBridges`, direct path first then the
bridge DFS) is shared by all three operations below.

### 4b. Disassembly & Assembly planners (paper §5.3)

Both are modeled as **sequences of single-module motions** and reuse
`routeWithBridges`. The whole neighborhood is treated as filled
(worst case), except the one partner slot along `V⃗` — the recipient for
Disassembly, the donor channel for Assembly — which stays open so the
slot is never a fully-enclosed (unreachable) pocket.

- **Disassembly(`V⃗`)** — greedily peel modules off `M`, one at a time,
  routing each to the staging `center`. A removable cell must keep the
  remaining set connected; the cheapest reachable one is removed first.
  Direct routes are tried for all candidates before any bridges, so the
  common case stays cheap.
- **Assembly(`V⃗`)** — greedily build a fresh `M` from `center`, one
  module at a time; each newly placed cell must attach to the partial
  structure. Same direct-first-then-bridges policy.

A meta-module is **feasible iff all 18 `(operation, direction)` pairs
succeed** — Transfer, Disassembly and Assembly in each of the 6
±-directions. `AllDirectionsCost::feasiblePairs` records how many of the
18 succeed (used by the fitness gradient below).

### 5. Genetic algorithm

Outer loop:

```
for size in [minSize .. maxSize]:
    for basis in defaultCandidateBases():
        runGA(basis, size)
        if any individual is feasible: return it
```

This bias guarantees the **smallest** feasible `|M|` is returned — the
first iteration of the outer loop that admits any solution wins. Within
a fixed `(size, basis)`, the GA runs:

- **Representation** — `CellSet` (a `std::set<Cell3DPosition>`).
  Shapes are kept connected and live inside a bounded box
  `[-radius..radius]³`.
- **Initialization** — random growth: seed at the origin, repeatedly
  attach a random FCC-frontier cell until `size` is reached.
- **Selection** — tournament of size `tournamentK` (default 3).
- **Crossover** — union of two parents, then BFS-prune from a random
  seed cell of parent A back down to `size` while preserving
  connectivity.
- **Mutation** — flip a coin: either drop a non-articulation cell or
  grow at a random frontier cell. A subsequent repair phase enforces
  exactly `size` connected cells.
- **Elitism** — best individual carried forward unchanged.
- **Fitness** —
  - infeasible (`< 18` feasible pairs):
    `-1e9 + 10⁶ · feasiblePairs − |M|`. The `feasiblePairs` term
    (0..18) is the gradient that lets the GA hill-climb from random
    shapes toward full feasibility instead of seeing a flat penalty;
    any infeasible still scores far below any fully-feasible one.
  - feasible (all 18 pairs): `-(10⁶ · |M| + total_moves)` where
    `total_moves = transfer + disassembly + assembly`. The huge weight
    on `|M|` is irrelevant inside one GA run (size is fixed) but
    makes the size-first outer loop's tiebreak unambiguous.
- **Termination** — the GA exits as soon as any feasible individual
  appears (size-first search).

### 6. Candidate bases

`defaultCandidateBases()` tries six bases by default, ordered roughly
by increasing fundamental-domain volume:

| Label | `X⃗` | `Y⃗` | `Z⃗` | Notes |
|---|---|---|---|---|
| `ortho-3-3-2` | (3,0,0) | (0,3,0) | (0,0,2) | Densest kept basis |
| `ortho-4-4-2` | (4,0,0) | (0,4,0) | (0,0,2) | |
| `cubic-4` | (4,0,0) | (0,4,0) | (0,0,4) | |
| `ortho-5-5-4` | (5,0,0) | (0,5,0) | (0,0,4) | More porous |
| `scaffold-flat` | (6,0,0) | (0,6,0) | (0,0,2) | Inspired by Thalamy et al. (needs larger `--radius`) |
| `hexagonal (paper)` | (8,0,0) | (4,3,0) | (4,1,4) | The hexagonal meta-module from the paper (needs larger `--radius`) |

> `cubic-2` (spacing 2) was removed from the defaults: it packs
> meta-modules solid, sealing the assembly slot under the OctaFace lock
> so **Assemble is infeasible in every direction**. Porosity — open
> channels between/inside sparse meta-modules — is what makes
> Dismantle/Transfer/Assemble simultaneously satisfiable, so the kept
> bases all leave space. Large bases (`scaffold-flat`, `hexagonal`) only
> admit valid shapes once `--radius` is big enough to host an axis
> contact across the basis vector.

To experiment with different bases, edit
`defaultCandidateBases()` in [metaModuleCore.cpp](metaModuleCore.cpp).

## Extending the generator

- **Add a new basis** — append to `defaultCandidateBases()` in
  [metaModuleCore.cpp](metaModuleCore.cpp).
- **Adopt the full Algorithm 1 bridge filter** — replace
  `possibleBridges` (currently the cheap 2-neighbor heuristic) with the
  paper's reachability-difference filter. Mind the BFS-bound budget
  per evaluation.
- **Different robot families** — swap `fccNeighbors`,
  `isTargetLocked`, and the default bases. The rest of the framework
  (validators, planners, GA) is geometry-agnostic.

## Known limitations

- Disassembly / Assembly are planned with a **greedy** elimination /
  build order and the cheap bridge heuristic. The order is not
  backtracked, so a meta-module that needs a non-greedy sequence may be
  reported infeasible even if one exists.
- Disassembly/Assembly route every module to/from a **single fixed
  `center`** and assume the worst-case filled neighborhood (minus the
  one partner slot). This is a model, not the paper's hand-coded
  sequences; costs are indicative move counts rather than exact plans.
- Bridge enumeration uses a heuristic (`≥ 2 C_piv neighbors`) rather
  than the full Algorithm 1 step-3 reachability differencing.
- The bounded box search (`--radius`) restricts shapes to a cube and
  may miss very thin meta-modules. Increase `--radius` for larger
  shapes (cost grows roughly with `(2R+1)³`).
- The motion predicate covers the OctaFace blocking rule only; HexaFace
  exceptions for special pivots are not modeled.
- Search is single-threaded. For larger sweeps consider running
  multiple seeds in parallel via shell job control.
