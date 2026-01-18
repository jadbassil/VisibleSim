#!/usr/bin/env python3
import sys
import shlex
import subprocess
from pathlib import Path

def main():
    base_dir = Path(__file__).parent.resolve()
    exe = base_dir / "diameterMonitoring"
    if not exe.is_file():
        sys.stderr.write(f"Missing executable: {exe}\n")
        sys.exit(1)

    # Config sizes to test
    sizes = [10, 20]
    for i in range(sizes[0], sizes[1] + 1):
        cfg = base_dir.parent / "diameterMonitoring" / "cube" / f"cube{i}.xml"
        if not cfg.is_file():
            sys.stderr.write(f"Missing config: {cfg}\n")
            sys.exit(1)

        cmd = [str(exe), "-c", str(cfg), "-t"]
        print("Running:", " ".join(shlex.quote(x) for x in cmd))
        try:
            result = subprocess.run(cmd, check=False)
        except FileNotFoundError as e:
            sys.stderr.write(f"Execution failed (not found): {e}\n")
            sys.exit(1)
        if result.returncode != 0:
            sys.stderr.write(f"Run failed with exit code {result.returncode}\n")
            sys.exit(result.returncode)

    print("All runs completed successfully.")

if __name__ == "__main__":
    main()