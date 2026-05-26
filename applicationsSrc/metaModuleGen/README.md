# metaModuleGen — automatic meta-module generator for 3D Catoms

A standalone VisibleSim application that searches for the **smallest** valid
meta-module satisfying the formal framework of Lamiroy, Bassil & Piranda
(*A Formal Framework for Meta-Module Design in Modular Robot-Based
Programmable Matter*) and minimizes total motion cost over the six
±-directions. The search runs a genetic algorithm; the underlying motion
predicate is implemented for the FCC lattice used by 3D Catoms.

The generator is fully decoupled from the live VisibleSim runtime so a
single fitness evaluation does not require booting the GLUT viewer or
instantiating `Catoms3DBlock` objects. Once the GA finishes, the best
shape is exported to a config XML and (optionally) handed to the standard
Catoms3D viewer for inspection.

## Files

| File | Role |
|---|---|
| [metaModuleCore.h](metaModuleCore.h) / [metaModuleCore.cpp](metaModuleCore.cpp) | Standalone FCC neighbor + `canMove` predicate, MetaModule type, geometric validators, bridge finder, Transfer planner, evaluator, GA |
| [metaModuleGen.cpp](metaModuleGen.cpp) | `main()`: CLI parsing, GA driver, XML export, optional viewer boot |
| [metaModuleGenBlockCode.h](metaModuleGenBlockCode.h) / [.cpp](metaModuleGenBlockCode.cpp) | Minimal `Catoms3DBlockCode` so the exported shape can be rendered |
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
| `--min-size=N` | 4 | Smallest `\|M\|` tested by the outer size loop |
| `--max-size=N` | 30 | Upper bound on `\|M\|` before giving up |
| `--radius=N` | 3 | Half-extent of the bounding box `[-N..N]³` cells in which the shape lives |
| `--pop=N` | 120 | GA population per `(basis, size)` |
| `--gens=N` | 200 | GA generations per `(basis, size)` |
| `--seed=N` | 42 | RNG seed |
| `--out-xml=PATH` | `metaModule_result.xml` | Where to write the exported VisibleSim config |

Any other flag (`-r`, `-R`, `-c file.xml`, `-t`, `-s`, …) is forwarded
unchanged to `createSimulator`. The application auto-injects `-r` if no
`-r/-R` is present so the viewer actually launches.

### Output

On success the binary prints the chosen basis, the cells of the
meta-module, the proposed `center`, and a per-direction cost breakdown:

```
*** Best meta-module ***
basis label : ortho-3-3-2
MetaModule { size=7 basis=( X=(3,0,0), Y=(0,3,0), Z=(0,0,2))
             center=(-1,-2,-1) cells={...} }
Transfer cost per direction:
    +X  feasible=yes  bridges=0  setup=0  perModule=3  clear=0  total=3
    -X  feasible=yes  bridges=0  setup=0  perModule=4  clear=0  total=4
    +Y  feasible=yes  bridges=0  setup=0  perModule=3  clear=0  total=3
    -Y  feasible=yes  bridges=0  setup=0  perModule=4  clear=0  total=4
    +Z  feasible=yes  bridges=0  setup=0  perModule=2  clear=0  total=2
    -Z  feasible=yes  bridges=0  setup=0  perModule=2  clear=0  total=2
total transfer moves (sum over ±X, ±Y, ±Z): 18
```

It also writes a VisibleSim-style config XML (default
`metaModule_result.xml`) that the viewer can ingest directly.

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

A direction is infeasible if neither variant produces a path. A
meta-module is feasible iff all 6 directions succeed.

> **Note:** Construction / Disassembly (paper §5.3) are not yet planned
> by the current implementation. The `AllDirectionsCost` struct is
> structured so a future change can plug them in.

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
  - infeasible (Transfer infeasible in any direction):
    `-1e9 − |M|` so infeasibles still gradient-flow toward feasible
    regions;
  - feasible: `-(10⁶ · |M| + total_transfer_moves)`. The huge weight
    on `|M|` is irrelevant inside one GA run (size is fixed) but
    makes the size-first outer loop's tiebreak unambiguous.
- **Termination** — the GA exits as soon as any feasible individual
  appears (size-first search).

### 6. Candidate bases

`defaultCandidateBases()` tries six bases by default, ordered roughly
by increasing fundamental-domain volume:

| Label | `X⃗` | `Y⃗` | `Z⃗` | Notes |
|---|---|---|---|---|
| `cubic-2` | (2,0,0) | (0,2,0) | (0,0,2) | Smallest possible |
| `ortho-3-3-2` | (3,0,0) | (0,3,0) | (0,0,2) | Where the size-7 solution is currently found |
| `ortho-4-4-2` | (4,0,0) | (0,4,0) | (0,0,2) | |
| `cubic-4` | (4,0,0) | (0,4,0) | (0,0,4) | |
| `scaffold-flat` | (6,0,0) | (0,6,0) | (0,0,2) | Inspired by Thalamy et al. |
| `hexagonal (paper)` | (8,0,0) | (4,3,0) | (4,1,4) | The hexagonal meta-module from the paper |

To experiment with different bases, edit
`defaultCandidateBases()` in [metaModuleCore.cpp](metaModuleCore.cpp).

## Extending the generator

- **Add a new basis** — append to `defaultCandidateBases()` in
  [metaModuleCore.cpp](metaModuleCore.cpp).
- **Plug in Construction / Disassembly** — implement `planConstruction`
  in [metaModuleCore.cpp](metaModuleCore.cpp), wire its cost into
  `AllDirectionsCost`, and update `scoreFitness`.
- **Adopt the full Algorithm 1 bridge filter** — replace
  `possibleBridges` (currently the cheap 2-neighbor heuristic) with the
  paper's reachability-difference filter. Mind the BFS-bound budget
  per evaluation.
- **Different robot families** — swap `fccNeighbors`,
  `isTargetLocked`, and the default bases. The rest of the framework
  (validators, planners, GA) is geometry-agnostic.

## Known limitations

- Construction / Disassembly cost is not yet computed; only Transfer
  feasibility & cost is checked. Meta-modules may therefore lack
  Construction/Disassembly sequences in practice.
- Bridge enumeration uses a heuristic (`≥ 2 C_piv neighbors`) rather
  than the full Algorithm 1 step-3 reachability differencing.
- The bounded box search (`--radius`) restricts shapes to a cube and
  may miss very thin meta-modules. Increase `--radius` for larger
  shapes (cost grows roughly with `(2R+1)³`).
- The motion predicate covers the OctaFace blocking rule only; HexaFace
  exceptions for special pivots are not modeled.
- Search is single-threaded. For larger sweeps consider running
  multiple seeds in parallel via shell job control.
