# Run the executable in ../../utilities/VisibleSimConfigGenerator/build/VisibleSimConfigGenerator to execute cubes of different sizes.

import subprocess
import os

# Path to the executable
executable_path = "../../utilities/VisibleSimConfigGenerator/build/VisibleSimConfigGenerator"

# Define cube sizes to generate
parameters = [3, 5, 7, 10, 15, 20]


# Ensure the executable exists
if not os.path.exists(executable_path):
    print(f"Error: Executable not found at {executable_path}")
    exit(1)

# Run the executable for each cube size
for size in parameters:
    output_file = os.path.join("ball", f"ball{size}.xml")
    print(f"Generating configuration for {size}...")
    try:
        result = subprocess.run(
            [executable_path, "-r", "blinkyblocks", "-o", output_file, "-t", "b", str(size)],
            capture_output=True,
            text=True,
            check=True
        )
        print(f"Success: Generated {output_file}")
        if result.stdout:
            print(result.stdout)
    except subprocess.CalledProcessError as e:
        print(f"Error cube size {size}: {e.stderr}")
