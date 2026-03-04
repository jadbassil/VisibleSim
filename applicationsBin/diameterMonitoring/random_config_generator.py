# Run the executable in ../../utilities/VisibleSimConfigGenerator/build/VisibleSimConfigGenerator to execute cubes of different sizes.

import subprocess
import os

# Path to the executable
executable_path = "../../utilities/VisibleSimConfigGenerator/build/VisibleSimConfigGenerator"

# Define cube sizes to generate
sizes = [50, 100, 150, 200]
occupancies = [0.2, 0.4, 0.6, 0.8]


# Ensure the executable exists
if not os.path.exists(executable_path):
    print(f"Error: Executable not found at {executable_path}")
    exit(1)

# Run the executable for each cube size
for trial in range(10):
    for size in sizes:
        for occupancy in occupancies:
            output_file = os.path.join("random_diameter_check", f"random_{size}_{int(occupancy*100)}_{trial}.xml")
            if not os.path.exists("random_diameter_check"):
                os.makedirs("random_diameter_check")
            print(f"Generating configuration for {size}, occupancy {occupancy}...")
            try:
                result = subprocess.run(
                    [executable_path, "-r", "bb", "-o", output_file, "-t", "r", str(size), "-d", str(occupancy)],
                    capture_output=True,
                    text=True,
                    check=True
                )
                print(f"Success: Generated {output_file}")
                if result.stdout:
                    print(result.stdout)
                if result.stderr:
                    print(result.stderr)
            except subprocess.CalledProcessError as e:
                print(f"Error cube size {size}, occupancy {occupancy}: {e.stderr}")
