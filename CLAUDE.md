# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What is VisibleSim

A C++17 discrete-event simulator for distributed algorithms on modular robots. Each module runs an independent copy of user-supplied "block code" (a distributed algorithm), communicates via P2P message passing, and can move within a 3D grid. The simulator renders in OpenGL or runs headless.

## Build Commands

### Using Make (primary)
```sh
make -j$(sysctl -n hw.ncpu)         # build all (core libs + all apps)
make test                            # run regression tests across all apps
make doc                             # generate Doxygen docs
make clean                           # remove build artifacts
make realclean                       # also remove generated docs
```

### Using CMake (preferred for IDEs)
```sh
cmake -B build .
make -j$(sysctl -n hw.ncpu) -C build
```

**macOS dependencies (Apple Silicon):** freeglut, glew, muparser via Homebrew at `/opt/homebrew`. See `MACOS_SETUP.md` for full setup.

### Building a single app
```sh
cd applicationsSrc/<appName>
make
```

## Running Simulations

```sh
# Convenience wrapper from project root:
./runBlockCode.sh <appName> [-c config.xml] [options]

# Or directly from the binary:
cd applicationsBin/<appName>
./<appName> -c config.xml -r        # graphical, realtime
./<appName> -c config.xml -t        # headless/terminal mode (-R -x implied)
./<appName> -c config.xml -R -x     # graphical, fastest, auto-exit
```

Key CLI flags: `-r`/`-R` realtime/fastest scheduler, `-x` auto-exit, `-t` terminal mode, `-c <file>` config, `-s [maxDate|inf]` scheduler stop, `-a <seed>` RNG seed, `-g` export final config for regression testing, `-l` log to `simulation.log`.

## Running a Single Regression Test
```sh
cd applicationsSrc/<appName>
make test
# Or manually:
../../utilities/blockCodeTest.sh <testID> ./<binary>
# This runs the binary with -t -g, then diffs confCheck.xml against .controlConf_<testID>.xml
```

## Code Architecture

### Simulation Lifecycle
1. `main()` calls `createSimulator(argc, argv, BlockCodeBuilder)`.
2. Simulator parses `config.xml` (TinyXML), instantiates `World`, `Lattice`, `Scheduler`, and one `BuildingBlock` + `BlockCode` per module. A `CodeStartEvent` at `t=0` is enqueued for every block.
3. `startSimulation()` links neighbor interfaces, starts the scheduler thread, enters the GLUT loop (or waits headless).
4. Scheduler dispatches `CodeStartEvent` → `startup()` on every block simultaneously. Algorithm runs via message passing.
5. Simulation ends when the event queue empties or max-date is reached.

### Key Singletons
- `Simulator` — config parsing, startup orchestration
- `World` — owns the block map (`map<bID, BuildingBlock*>`), lattice, camera, and GL rendering
- `Scheduler` — discrete-event engine: `multimap<Time, EventPtr>` (time in microseconds)

### Directory Structure
```
simulatorCore/src/
  base/       Core abstractions: BuildingBlock, BlockCode, World, Simulator, GlBlock
  events/     Scheduler and event base classes
  comm/       Message, MessageOf<T>, P2PNetworkInterface
  grid/       Lattice (spatial grid) and Target (reconfiguration goal)
  gui/        OpenGL viewer, camera, shaders, obj loader
  motion/     Translation and teleportation events
  math/       Cell3DPosition (integer grid coords), Vector3D, Matrix44
  clock/      Per-module simulated clocks (with optional noise)
  csg/        Constructive Solid Geometry for shape specification
  stats/      Per-module and global simulation statistics
  replay/     Replay export
  utils/      CLI parser, trace/logging, color, config exporter, tDefs.h
  robots/     Per-robot-family implementations (see below)
  deps/       TinyXML (XML parser), Eigen (linear algebra)

applicationsSrc/<appName>/   User application source
applicationsBin/<appName>/   Compiled binaries and config.xml files
```

### Robot Families (`simulatorCore/src/robots/`)
Each family (`blinkyBlocks`, `catoms3D`, `hexanodes`, `slidingCubes`, `smartBlocks`, `datoms`) provides:
- `<robot>Block` — concrete `BuildingBlock` subclass (interface count, motion)
- `<robot>BlockCode` — abstract `BlockCode` base for user apps to extend
- `<robot>World` / `<robot>Simulator` — world/simulator subclasses with family-specific XML parsing and GL drawing
- `<robot>MotionEngine` / `<robot>MotionRules` — kinematics and move validation (motion-capable robots)
- `<robot>*Events` — robot-specific event types

Core build output: `simulatorCore/lib/libsim<RobotFamily>.a` (one static lib per family).

### Writing a User Application

Every application follows this pattern:

**`<appName>.cpp`** — `main()`:
```cpp
#include "robots/blinkyBlocks/blinkyBlocksSimulator.h"
#include "<appName>BlockCode.h"

int main(int argc, char **argv) {
    createSimulator(argc, argv, MyBlockCode::buildNewBlockCode);
    getSimulator()->printInfo();
    deleteSimulator();
    return 0;
}
```

**`<appName>BlockCode.h/.cpp`** — user algorithm:
```cpp
class MyBlockCode : public BlinkyBlocksBlockCode {
public:
    MyBlockCode(BlinkyBlocksBlock *host) : BlinkyBlocksBlockCode(host) {}
    void startup() override;   // register handlers, initiate algorithm
    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return new MyBlockCode((BlinkyBlocksBlock*)host);
    }
};
```

In `startup()`: register handlers via `addMessageEventFunc2`, then start the algorithm (e.g., seed block sends first message). Use `sendMessage(msg, interface, t0, dt)` or `sendMessageToAllNeighbors(...)`.

**`applicationsBin/<appName>/config.xml`** — world description (grid size, camera, block positions, optional target config for reconfiguration apps).

**`applicationsSrc/<appName>/Makefile`** — set `SRCS`, `OUT`, `MODULELIB` (e.g., `-lsimBlinkyBlocks`), and `TESTS` for regression test ID.

### Core API (in `blockCode.h`)
- `startup()` — called at `t=0`, pure virtual, entry point for distributed algorithm
- `addMessageEventFunc2(type, handler)` — register a typed message handler
- `sendMessage(msg, interface, t0, dt)` — send with simulated delay
- `sendMessageToAllNeighbors(msg, t0, dt, excludeInterfaces...)` — broadcast
- `onMotionEnd()`, `onNeighborChanged()`, `onTap()`, `onGlDraw()` — optional override hooks
- `scheduler->now()` — current simulation time (microseconds)
- `hostBlock->position` — block's `Cell3DPosition` in the grid
- `hostBlock->blockId` — unique `bID`

### Coding Style (from `doc/TechnicalReference.md`)
- K&R brace style, 4-space indentation, 80-character line limit
- C++17; no raw owning pointers where smart pointers apply
- Doxygen comments for all public API
