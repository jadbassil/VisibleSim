#!/usr/bin/env python3
import sys
import re
import shlex
import subprocess
import csv
import argparse
from pathlib import Path

def parse_output(stdout_text, stderr_text):
    """Parse relevant values from the program output."""
    results = {}
    
    # Strip ANSI escape codes (color formatting)
    ansi_escape = re.compile(r'\x1b\[[0-9;]*m')
    stdout_text = ansi_escape.sub('', stdout_text)
    stderr_text = ansi_escape.sub('', stderr_text)
    
    # Parse from STDOUT
    stdout_patterns = {
        'num_robots': r'Number of robots:\s+(\d+)',
        'simulator_time_us': r'Simulator elapsed time:\s+([\d.]+)\s+us',
        'num_messages': r'Number of messages processed:\s+(\d+)',
    }
    
    for key, pattern in stdout_patterns.items():
        match = re.search(pattern, stdout_text, re.IGNORECASE)
        if match:
            results[key] = match.group(1)
    
    # Parse from STDERR
    stderr_patterns = {
        'diameter': r'diameter\s+(\d+)',
    }
    
    for key, pattern in stderr_patterns.items():
        match = re.search(pattern, stderr_text, re.IGNORECASE)
        if match:
            results[key] = match.group(1)
    
    return results

def save_results_to_csv(results, output_file='results.csv'):
    """Save parsed results to a CSV file for later plotting with matplotlib."""
    if not results:
        print("No results to save.")
        return
    
    # Get all unique keys from all result dictionaries
    fieldnames = set()
    for res in results:
        fieldnames.update(res.keys())
    fieldnames = sorted(fieldnames)  # Sort for consistent column order
    
    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(results)
    
    print(f"\nResults saved to {output_file}")

def main():
    parser = argparse.ArgumentParser(description='Run diameter monitoring simulations')
    parser.add_argument('-c', '--config', type=str, default='cube',
                        help='Config folder name (default: cube)')
    args = parser.parse_args()
    
    base_dir = Path(__file__).parent.resolve()
    exe = base_dir / "diameterMonitoring"
    if not exe.is_file():
        sys.stderr.write(f"Missing executable: {exe}\n")
        sys.exit(1)

    # Get config directory and find all XML files
    config_name = args.config
    config_dir = base_dir.parent / "diameterMonitoring" / config_name
    
    if not config_dir.is_dir():
        sys.stderr.write(f"Config directory not found: {config_dir}\n")
        sys.exit(1)
    
    # Find all XML files in the config directory
    xml_files = sorted(config_dir.glob("*.xml"))
    
    if not xml_files:
        sys.stderr.write(f"No XML files found in {config_dir}\n")
        sys.exit(1)
    
    print(f"Found {len(xml_files)} XML files in {config_dir}")
    all_results = []
    
    for cfg in xml_files:
        print(cfg)
        cmd = [str(exe), "-c", str(cfg), "-t"]
        print("Running:", " ".join(shlex.quote(x) for x in cmd))
        try:
            result = subprocess.run(
                cmd, 
                capture_output=True,  # Capture stdout and stderr
                text=True,            # Decode as text
                check=False
            )
            
            # Print the output
            if result.stdout:
                print("STDOUT:")
                print(result.stdout)
            if result.stderr:
                print("STDERR:")
                print(result.stderr)
            
            # Parse the output (pass both stdout and stderr)
            print(f"\n=== DEBUG: stdout length = {len(result.stdout)}, stderr length = {len(result.stderr)} ===")
            parsed = parse_output(result.stdout, result.stderr)
            print(f"Parsed: {parsed}")
            parsed['config_file'] = cfg.name
            # Extract size from filename if possible (e.g., cube10.xml -> 10)
            size_match = re.search(r'(\d+)', cfg.stem)
            if size_match:
                parsed['size'] = int(size_match.group(1))
            # parsed['return_code'] = result.returncode
            all_results.append(parsed)
            
            print(f"Parsed results: {parsed}")
            
        except FileNotFoundError as e:
            sys.stderr.write(f"Execution failed (not found): {e}\n")
            sys.exit(1)
            
        if result.returncode != 0:
            sys.stderr.write(f"Run failed with exit code {result.returncode}\n")
            sys.exit(result.returncode)

    # Summary of all results
    print("\n=== Summary ===")
    for res in all_results:
        print(f"Size {res.get('size')}: {res}")
    
    # Save results to CSV file
    save_results_to_csv(all_results, f'simulation_results_{config_name}.csv')

if __name__ == "__main__":
    main()