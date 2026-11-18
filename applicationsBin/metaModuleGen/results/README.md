# metaModuleGen results — 2026-11-04

First runs of the exhaustive (`enum`) and MAP-Elites (`mapelites`) searches.
Code: branch `MM-generation` at commit `71d5f796`, plus the uncommitted
`metaModuleSearch.{h,cpp}` added that day.

All costs use the **original move model** in `metaModuleCore.cpp`. Under that
model one move is a hop to any free neighbor of an adjacent pivot, which can
be more than one real Catom rotation. Transfer is charged for one module,
while disassembly and assembly are charged for every module. Regenerate these
files if the move model changes.

## Files

One folder per run. `enum_ortho-3-3-2/` and `mapelites_20k/` were written by
`--save-dir` and contain:

- one structure XML per feasible design, named
  `<rank>_size<N>_<basis>_<moves>moves.xml` and ranked by size, then moves;
- `designs.csv`: one row per XML, with transfer / disassembly / assembly / total moves, center cell and cells;
- `run.json`: the settings and basis vectors used;
- `search.log`: the search's terminal output.

| Folder | Designs | Extra files |
|---|---|---|
| `enum_ortho-3-3-2/` | All 10 feasible 6-module shapes (212–280 moves) | |
| `mapelites_20k/` | The 4 feasible elites: sizes 6, 7, 8 on ortho-3-3-2 and 9 on ortho-4-4-2 | `archive.csv`: best shape of all 24 niches, feasible or not |
| `mapelites_3k/` | The best design of the end-to-end check | `archive.csv` (no center column; written before it existed) |

`designs_viewer.html` is a standalone 3D viewer of all 13 feasible designs.
It needs internet to load three.js.

Open any design in the VisibleSim viewer, from `applicationsBin/metaModuleGen`:

```sh
./metaModuleGen --view=results/enum_ortho-3-3-2/01_size6_ortho-3-3-2_212moves.xml
```

Cells in the CSVs are grid coordinates (`x y z`, cells separated by `;`). The
XMLs tile each design 2×2×2 and translate it into the positive octant, so
their coordinates differ from the CSVs.

## Run settings

The `enum_ortho-3-3-2/` and `mapelites_20k/` folders were regenerated with an
`-O2` build of the app and `--save-dir`, using the settings below. The
regenerated feasible shapes, costs and archives match the first runs exactly,
so both searches are deterministic for a fixed seed and settings.

**enum**
- basis `ortho-3-3-2` = X (3,0,0), Y (0,3,0), Z (0,0,2)
- sizes 4..10, stopping after the first feasible size; max extent 9 (`--radius=4`); 14 threads; about 125 s
- command: `./metaModuleGen --no-sim --search=enum --bases=0 --min-size=4 --max-size=10 --save-dir=results/enum_ortho-3-3-2`

**mapelites, 20k**
- bases 0–3 (`ortho-3-3-2`, `ortho-4-4-2`, `cubic-4`, `ortho-5-5-4`); sizes 4..9; max extent 9
- 500 random seeds + 20,000 children; batch 64; basis-switch rate 0.1; tournament rate 0.5; seed 42; 14 threads; about 470 s
- command: `./metaModuleGen --no-sim --search=mapelites --bases=0,1,2,3 --min-size=4 --max-size=9 --iters=20000 --save-dir=results/mapelites_20k`

**mapelites, 3k** (real binary, Debug build)
- `./metaModuleGen --no-sim --search=mapelites --bases=0 --min-size=6 --max-size=7 --iters=3000 --init=200`

## Findings

- **ortho-3-3-2:** no shape of 4 or 5 modules is feasible (exhaustive, within the model and an extent of 9). There are exactly 10 feasible 6-module shapes, costing 212–280 moves.
- The two 212-move shapes give the same tiling: three cells differ by one Z basis vector.
- The 219-move shape has the same cost split (30 / 98 / 91) as the GA example in `applicationsSrc/metaModuleGen/README.md`.
- MAP-Elites found feasible designs at larger sizes. The 7-module one (207 moves) is the cheapest design found overall:

  | Size | Basis | Moves (T / D / A) | Center | Cells |
  |---|---|---|---|---|
  | 7 | ortho-3-3-2 | 207 (12 / 106 / 89) | (0,-1,-1) | `0 0 1;1 0 0;1 2 0;2 0 0;2 0 1;2 1 1;2 2 0` |
  | 8 | ortho-3-3-2 | 249 (14 / 126 / 109) | (0,-1,-1) | `0 0 1;1 0 0;1 1 1;1 2 0;2 0 0;2 0 1;2 1 1;2 2 2` |
  | 9 | ortho-4-4-2 | 324 (18 / 173 / 133) | (-1,-1,-1) | `0 0 0;1 0 0;1 1 0;2 -1 1;2 0 1;2 1 0;2 2 0;2 3 0;3 0 1` |

- MAP-Elites's 6-module elite (280 moves) is the most expensive of the 10 shapes the enumeration found. For the smallest size, rely on the exhaustive search.
