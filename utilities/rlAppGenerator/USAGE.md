# rlAppGenerator — VisibleSim RL training-app generator

`rlgen.py` scaffolds a VisibleSim application that exposes the simulation as a
reinforcement-learning **environment**. The generated app:

* opens a TCP socket and streams per-module observations to an external Python
  trainer as newline-delimited JSON, and applies the actions it sends back;
* ships a NumPy training stack (`env.py`, `model.py`, `train.py`) and a weight
  exporter that writes a compact, CRC-checked binary file;
* compiles out of the box for any robot family (motion is left as guided TODO
  hooks, so nothing family-specific blocks the build).

It is the RL-oriented sibling of `utilities/blockCodeGenerator/bcgen.py` and uses
the same `<<placeholder>>` substitution over the files in `sample/`.

## Quick start

```sh
cd utilities/rlAppGenerator
python3 rlgen.py -a MyAgent --register      # SlidingCubes by default
make -C ../../applicationsSrc myAgent        # build it
```

Then follow the generated `applicationsSrc/myAgent/USAGE.md` to train and export.

## Command-line options

| Flag | Default | Meaning |
|------|---------|---------|
| `-a, --app` | *(required)* | application name (e.g. `MyAgent`). Folder/files use the lower-camel form (`myAgent`). |
| `-m, --module` | `SlidingCubes` | robot family. One of: `Catoms3D`, `Catoms2D`, `Datoms`, `BlinkyBlocks`, `SmartBlocks`, `SlidingCubes`, `Nodes2D`, `Hexanodes`. |
| `-p, --port` | `9999` | TCP port for the gym socket bridge. |
| `-r, --register` | off | insert the app into the `SUBDIRS` line of `applicationsSrc/Makefile`. |

## What it generates

```
applicationsSrc/<app>/
  <app>.cpp                 main()
  <app>BlockCode.hpp/.cpp   environment logic + TODO hooks (moves/reward/done)
  gymServer.hpp/.cpp        generic TCP bridge (POSIX sockets, no edits needed)
  Makefile                  MODULELIB = -lsim<Family>, links pthread
  USAGE.md                  per-app build/train/protocol/export/deploy guide
  train/
    env.py                  gym-style wrapper (subprocess + socket + obs->arrays)
    model.py                NumPy MLP policy
    train.py                minimal REINFORCE loop, periodic weight export
    export_weights.py       VSW1 binary writer + CRC-verifying loader
    requirements.txt        numpy
applicationsBin/<app>/config.xml   starter world for the chosen family
```

The generator refuses to overwrite an existing `applicationsSrc/<app>` or
`applicationsBin/<app>` directory.

## Design notes

* **Coordinator.** The lowest-id module owns the socket and drives a periodic
  "gym tick" (an `InterruptionEvent`); it builds the whole-assembly observation,
  sends it, waits for an action line, applies it, and repeats. Other modules just
  initialise. The binary runs until the trainer disconnects/kills it — the Python
  `reset()` relaunches a fresh process per episode.
* **Separation of concerns.** `gymServer.*` is pure POSIX + `std` (reusable);
  all VisibleSim-specific observation building and action dispatch lives in the
  block code. Swapping ML frameworks touches only `train/`.
* **Weights are a contract.** The `VSW1` layout (magic + version + named tensors
  with shapes + `float32` data + `zlib.crc32`) is documented in the per-app
  `USAGE.md` so a future C++ deploy/inference path can load it unchanged.

## Customising the templates

Templates live in `sample/`. Edit them to change what every newly generated app
looks like. Available placeholders:

| Placeholder | Example value |
|-------------|---------------|
| `<<appName>>` | `MyAgent` |
| `<<appNameLc>>` | `myAgent` |
| `<<moduleName>>` | `SlidingCubes` |
| `<<moduleNameLc>>` | `slidingCubes` |
| `<<gymPort>>` | `9999` |

`sample/config.xml` is special-cased to land in `applicationsBin/<app>/`;
everything else is written under `applicationsSrc/<app>/`, with `sample.cpp` /
`sampleBlockCode.*` renamed after the application.

## Framework integration & deployment

Each generated app's `USAGE.md` contains full sections on (a) replacing the NumPy
trainer with PyTorch / JAX / Gymnasium / Stable-Baselines3 while keeping the same
weight format, and (b) loading `weights.bin` in C++ for on-robot inference
(reference loader + where to wire it). Start there once your app is generated.
