#!/usr/bin/env python3
"""
rlgen.py - VisibleSim RL training-application skeleton generator.

Generates a ready-to-build VisibleSim application that exposes the simulation as
a reinforcement-learning *environment*: the C++ side opens a TCP socket and
streams per-module observations to an external (NumPy) Python trainer as
newline-delimited JSON, receives actions back, and the trained policy is
exported to a compact binary weight file.

This is the RL-oriented sibling of utilities/blockCodeGenerator/bcgen.py: it
reuses the same <<placeholder>> substitution approach over the files in
sample/, but emits the additional socket-bridge + training scaffolding.

Usage:
    python3 rlgen.py -a MyAgent [-m SlidingCubes] [-p 9999] [--register]

See USAGE.md for the full walkthrough.
"""

import os
import sys
import argparse

# The robot families VisibleSim ships a -lsim<family> library for.
MODULE_OPTIONS = ["Catoms3D", "Catoms2D", "Datoms", "BlinkyBlocks",
                  "SmartBlocks", "SlidingCubes", "Nodes2D", "Hexanodes"]


def make_dir(path):
    """Create a directory (and parents); abort if it already exists as a dir."""
    if os.path.isdir(path):
        print("Directory already exists, aborting to avoid overwrite: %s" % path)
        sys.exit(1)
    try:
        os.makedirs(path)
    except OSError as e:
        print("Creation of the directory %s failed: %s" % (path, e))
        sys.exit(1)
    print("Created directory %s" % path)


def substitute(text, subs):
    """Apply every <<placeholder>> -> value replacement in `subs` to `text`."""
    for tag, value in subs.items():
        text = text.replace(tag, value)
    return text


def create_file(template_path, out_path, subs):
    """Read a template, substitute placeholders, write the result."""
    with open(template_path, "rt") as fin:
        content = fin.read()
    out_dir = os.path.dirname(out_path)
    if out_dir and not os.path.isdir(out_dir):
        os.makedirs(out_dir)
    with open(out_path, "wt") as fout:
        fout.write(substitute(content, subs))
    print("  wrote %s" % out_path)


def map_dest(rel_path, app_lc, src_path, bin_path):
    """Map a path relative to sample/ to its destination on disk.

    - config.xml goes next to the binary (applicationsBin/<app>/config.xml)
    - sample.cpp / sampleBlockCode.* are renamed after the application
    - everything else (gymServer.*, Makefile, USAGE.md, train/*) keeps its name
    """
    name = os.path.basename(rel_path)
    if name == "config.xml":
        return os.path.join(bin_path, "config.xml")

    rel = rel_path
    if name == "sample.cpp":
        rel = os.path.join(os.path.dirname(rel_path), app_lc + ".cpp")
    elif name == "sampleBlockCode.hpp":
        rel = os.path.join(os.path.dirname(rel_path), app_lc + "BlockCode.hpp")
    elif name == "sampleBlockCode.cpp":
        rel = os.path.join(os.path.dirname(rel_path), app_lc + "BlockCode.cpp")
    return os.path.join(src_path, rel)


def register_app(app_src_dir, app_lc):
    """Insert <app_lc> into the SUBDIRS = ... line of applicationsSrc/Makefile."""
    makefile = os.path.join(app_src_dir, "Makefile")
    if not os.path.isfile(makefile):
        print("WARNING: %s not found, cannot auto-register." % makefile)
        return
    with open(makefile, "rt") as f:
        lines = f.readlines()
    for i, line in enumerate(lines):
        if line.startswith("SUBDIRS"):
            apps = line.split("=", 1)[1].split()
            if app_lc in apps:
                print("Already registered in applicationsSrc/Makefile.")
                return
            lines[i] = line.rstrip("\n") + " " + app_lc + "\n"
            with open(makefile, "wt") as f:
                f.writelines(lines)
            print("Registered '%s' in applicationsSrc/Makefile SUBDIRS." % app_lc)
            return
    print("WARNING: no SUBDIRS line found in %s; please register manually." % makefile)


def main():
    parser = argparse.ArgumentParser(
        description="Generate a VisibleSim RL training-environment application "
                    "(socket bridge to an external Python trainer + weight export).")
    parser.add_argument("-a", "--app", type=str, required=True,
                        help="name of the application to create (e.g. MyAgent)")
    parser.add_argument("-m", "--module", type=str, default="SlidingCubes",
                        choices=MODULE_OPTIONS,
                        help="target robot family (default: SlidingCubes)")
    parser.add_argument("-p", "--port", type=int, default=9999,
                        help="TCP port for the gym socket bridge (default: 9999)")
    parser.add_argument("-r", "--register", action="store_true",
                        help="add the app to applicationsSrc/Makefile SUBDIRS")
    args = parser.parse_args()

    # Normalise names: AppName (PascalCase-ish) and appName (lowerCamel).
    app_name = args.app[0].upper() + args.app[1:]
    app_lc = app_name[0].lower() + app_name[1:]
    module = args.module
    module_lc = module[0].lower() + module[1:]

    subs = {
        "<<appName>>": app_name,
        "<<appNameLc>>": app_lc,
        "<<moduleName>>": module,
        "<<moduleNameLc>>": module_lc,
        "<<gymPort>>": str(args.port),
    }

    print("Generating RL application '%s' for family '%s' (port %d)\n"
          % (app_name, module, args.port))

    # Resolve paths relative to this script, so it works from any CWD.
    pkg_dir = os.path.dirname(os.path.abspath(__file__))
    sample_dir = os.path.join(pkg_dir, "sample")
    vs_root = os.path.dirname(os.path.dirname(pkg_dir))  # utilities/.. -> repo root
    app_src_root = os.path.join(vs_root, "applicationsSrc")
    src_path = os.path.join(app_src_root, app_lc)
    bin_path = os.path.join(vs_root, "applicationsBin", app_lc)

    if not os.path.isdir(sample_dir):
        print("ERROR: template directory not found: %s" % sample_dir)
        sys.exit(1)

    make_dir(src_path)
    make_dir(bin_path)

    # Walk the template tree and emit every file with substitution applied.
    for root, _dirs, files in os.walk(sample_dir):
        for fname in files:
            template_path = os.path.join(root, fname)
            rel_path = os.path.relpath(template_path, sample_dir)
            out_path = map_dest(rel_path, app_lc, src_path, bin_path)
            create_file(template_path, out_path, subs)

    print("\nApplication '%s' generated:" % app_name)
    print("  sources : %s" % src_path)
    print("  config  : %s" % os.path.join(bin_path, "config.xml"))
    print("  trainer : %s" % os.path.join(src_path, "train"))

    if args.register:
        register_app(app_src_root, app_lc)
    else:
        print("\nNext: add '%s' to the SUBDIRS line of applicationsSrc/Makefile "
              "(or re-run with --register), then build with `make`." % app_lc)

    print("\nSee %s for build + training instructions."
          % os.path.join(src_path, "USAGE.md"))


if __name__ == "__main__":
    main()
