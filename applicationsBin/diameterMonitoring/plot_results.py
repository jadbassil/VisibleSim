#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import argparse
from pathlib import Path

def plot_single_results(csv_file, vertical=False):
    """Plot number of messages and simulator time from CSV results.
    
    Args:
        csv_file: Path to CSV file
        vertical: If True, stack plots vertically; if False, arrange horizontally
    """
    
    # Read CSV file
    try:
        df = pd.read_csv(csv_file)
    except FileNotFoundError:
        print(f"Error: File not found: {csv_file}")
        return
    except pd.errors.EmptyDataError:
        print(f"Error: CSV file is empty: {csv_file}")
        return
    
    # Sort by size if available
    if 'size' in df.columns:
        df = df.sort_values('size')
        x_label = 'Size'
        x_data = df['size'].astype(str)
    else:
        x_label = 'Configuration'
        x_data = df['config_file'].astype(str) if 'config_file' in df.columns else range(len(df))
    
    # Create figure with subplots
    if vertical:
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(3.5, 6))
    else:
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(7, 3))
    
    # Plot 1: Number of Messages
    if 'num_messages' in df.columns:
        ax1.plot(range(len(x_data)), df['num_messages'] / 1e5, color='steelblue', linewidth=1.5, marker='o', markersize=5)
        ax1.set_xlabel(x_label, fontsize=9)
        ax1.set_ylabel('Number of Messages (×10⁵)', fontsize=9)
        ax1.set_title('Number of Messages per Configuration', fontsize=10, fontweight='bold')
        ax1.set_xticks(range(len(x_data)))
        ax1.set_xticklabels(x_data, rotation=45)
        ax1.tick_params(labelsize=8)
        ax1.grid(True, alpha=0.3)
    else:
        ax1.text(0.5, 0.5, 'num_messages column not found', ha='center', va='center')
        ax1.set_xticks([])
        ax1.set_yticks([])
    
    # Plot 2: Simulator Time
    if 'simulator_time_us' in df.columns:
        # Convert microseconds to milliseconds
        simulator_time_ms = df['simulator_time_us'] / 1000
        ax2.plot(range(len(x_data)), simulator_time_ms, color='coral', linewidth=1.5, marker='o', markersize=5)
        ax2.set_xlabel(x_label, fontsize=9)
        ax2.set_ylabel('Time (ms)', fontsize=9)
        ax2.set_title('Time per Configuration', fontsize=10, fontweight='bold')
        ax2.set_xticks(range(len(x_data)))
        ax2.set_xticklabels(x_data, rotation=45)
        ax2.tick_params(labelsize=8)
        ax2.grid(True, alpha=0.3)
    else:
        ax2.text(0.5, 0.5, 'simulator_time_us column not found', ha='center', va='center')
        ax2.set_xticks([])
        ax2.set_yticks([])
    
    plt.tight_layout()
    
    # Save figure
    output_file = Path(csv_file).stem + '_plots.pdf'
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Plot saved to {output_file}")
    
    # Show plot
    plt.show()

def plot_comparison(csv_files, vertical=False):
    """Create separate figures for each configuration.
    
    Args:
        csv_files: List of CSV file paths
        vertical: If True, stack plots vertically; if False, arrange horizontally
    """
    
    # Load all CSV files
    dfs = {}
    
    for csv_file in csv_files:
        try:
            df = pd.read_csv(csv_file)
            # Extract config name from filename (e.g., simulation_results_cube.csv -> cube)
            config_name = Path(csv_file).stem.replace('simulation_results_', '')
            dfs[config_name] = df
        except FileNotFoundError:
            print(f"Warning: File not found: {csv_file}")
        except pd.errors.EmptyDataError:
            print(f"Warning: CSV file is empty: {csv_file}")
    
    if not dfs:
        print("Error: No valid CSV files loaded")
        return
    
    # Create separate figure for each configuration
    for config_name, df in dfs.items():
        if 'num_robots' in df.columns:
            df = df.sort_values('num_robots')
            # Create x-axis labels with both num_robots and diameter
            if 'diameter' in df.columns:
                x_labels = [f"{int(robots)}\n(d={int(diam)})" for robots, diam in zip(df['num_robots'], df['diameter'])]
            else:
                x_labels = [str(int(robots)) for robots in df['num_robots']]
            x_data = range(len(df))
            x_label = 'Number of Robots (Diameter)'
        elif 'size' in df.columns:
            df = df.sort_values('size')
            x_data = df['size'].astype(str)
            x_labels = x_data
            x_label = 'Size'
        else:
            x_data = range(len(df))
            x_labels = df['config_file'].astype(str) if 'config_file' in df.columns else [str(i) for i in x_data]
            x_label = 'Configuration'
        
        # Create figure with subplots
        if vertical:
            fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(3.5, 6))
        else:
            fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(7, 3))
        # fig.suptitle(f'{config_name.capitalize()} Configuration', fontsize=16, fontweight='bold', y=1.02)
        
        # Plot 1: Number of Messages
        if 'num_messages' in df.columns:
            ax1.plot(x_data, df['num_messages'].values / 1e5, color='steelblue', linewidth=1.5, marker='o', markersize=5)
            ax1.set_xlabel(x_label, fontsize=9)
            ax1.set_ylabel('Number of Messages (×10⁵)', fontsize=9)
            ax1.set_title('Number of Messages', fontsize=10, fontweight='bold')
            ax1.set_xticks(x_data)
            ax1.set_xticklabels(x_labels, rotation=0, fontsize=8)
            ax1.grid(True, alpha=0.3)
        else:
            ax1.text(0.5, 0.5, 'num_messages column not found', ha='center', va='center')
            ax1.set_xticks([])
            ax1.set_yticks([])
        
        # Plot 2: Simulator Time
        if 'simulator_time_us' in df.columns:
            # Convert microseconds to milliseconds
            simulator_time_ms = df['simulator_time_us'].values / 1000
            ax2.plot(x_data, simulator_time_ms, color='coral', linewidth=1.5, marker='o', markersize=5)
            ax2.set_xlabel(x_label, fontsize=9)
            ax2.set_ylabel('Time (ms)', fontsize=9)
            ax2.set_title('Time', fontsize=10, fontweight='bold')
            ax2.set_xticks(x_data)
            ax2.set_xticklabels(x_labels, rotation=0, fontsize=8)
            ax2.grid(True, alpha=0.3)
        else:
            ax2.text(0.5, 0.5, 'simulator_time_us column not found', ha='center', va='center')
            ax2.set_xticks([])
            ax2.set_yticks([])
        
        plt.tight_layout()
        
        # Save figure
        output_file = f'{config_name}_plots.pdf'
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"Plot saved to {output_file}")
        
        # Show plot
        plt.show()

def main():
    parser = argparse.ArgumentParser(description='Plot simulation results')
    parser.add_argument('csv_files', nargs='*', 
                        help='CSV files to plot (if multiple, creates comparison; if single or none, uses default)')
    parser.add_argument('--vertical', '-v', action='store_true',
                        help='Stack plots vertically instead of horizontally (better for single-column figures)')
    args = parser.parse_args()
    
    if len(args.csv_files) > 1:
        # Multiple files - create comparison
        plot_comparison(args.csv_files, vertical=args.vertical)
    elif len(args.csv_files) == 1:
        # Single file
        plot_single_results(args.csv_files[0], vertical=args.vertical)
    else:
        # Default - compare all three
        default_files = ['simulation_results_cube.csv', 'simulation_results_ball.csv', 'simulation_results_random.csv']
        existing_files = [f for f in default_files if Path(f).exists()]
        
        if len(existing_files) > 1:
            print(f"Comparing: {', '.join(existing_files)}")
            plot_comparison(existing_files, vertical=args.vertical)
        else:
            # Fallback to single plot
            plot_single_results('simulation_results_cube.csv', vertical=args.vertical)

if __name__ == "__main__":
    main()
