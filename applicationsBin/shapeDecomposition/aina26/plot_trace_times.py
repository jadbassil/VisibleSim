#!/usr/bin/env python3
"""Generate summary analysis PDFs from aina26_results_*.json files.

Outputs always generated (unless files missing):
    scaling.pdf           - Phase 1 time & average trace time vs cube size
    scaling_messages.pdf  - Phase 1 messages & average trace messages vs cube size
    config_comparison.pdf - 4 bar charts comparing config3d / configFilled / configHoles

Usage:
    python3 plot_trace_times.py [--dir PATH] [--save]

Options:
    --dir   Path to directory containing aina26_results_*.json (default: script directory)
    --save  Write PDFs to disk (default). If omitted, figures are shown interactively.

Times in JSON (microseconds) are converted to seconds for plotting.
"""
from __future__ import annotations
import json
import argparse
from pathlib import Path
import statistics as stats
import math
import re
import matplotlib.pyplot as plt
import seaborn as sns

sns.set_context("talk")
sns.set_style("whitegrid")

def load_json(path: Path):
    with path.open("r") as f:
        return json.load(f)

def compute_trace_stats(recorded_times):
    """Compute stats from recorded times (in microseconds), converting to seconds."""
    if not recorded_times:
        return {
            "initial_phase_time": math.nan,
            "trace_times": [],
            "average_trace_time": math.nan,
            "total_time": math.nan
        }
    # Convert microseconds to seconds
    times_s = [t / 1e6 for t in recorded_times]
    initial_phase = times_s[0]
    trace_times = [times_s[i] - times_s[i-1] for i in range(1, len(times_s))]
    avg_trace = stats.mean(trace_times) if trace_times else math.nan
    total_time = times_s[-1]
    return {
        "initial_phase_time": initial_phase,
        "trace_times": trace_times,
        "average_trace_time": avg_trace,
        "total_time": total_time
    }

def compute_message_stats(recorded_messages):
    """Compute stats from recorded message counts."""
    if not recorded_messages:
        return {
            "initial_phase_messages": math.nan,
            "trace_messages": [],
            "average_trace_messages": math.nan,
            "total_messages": math.nan
        }
    initial_phase = recorded_messages[0]
    trace_msgs = [recorded_messages[i] - recorded_messages[i-1] for i in range(1, len(recorded_messages))]
    avg_trace = stats.mean(trace_msgs) if trace_msgs else math.nan
    total_msgs = recorded_messages[-1]
    return {
        "initial_phase_messages": initial_phase,
        "trace_messages": trace_msgs,
        "average_trace_messages": avg_trace,
        "total_messages": total_msgs
    }

# Removed per-file detailed plotting (plot_single) to keep only summary PDFs.


# Removed aggregate distribution computation.


# Removed aggregate plotting.


def extract_cube_size(filename):
    """Extract cube size from filename like 'aina26_results_cube10.json' -> 10"""
    m = re.search(r"cube(\d+)", filename)
    return int(m.group(1)) if m else None


def plot_scaling(files_stats, save=False, out_prefix="scaling"):
    """Plot phase 1 time and avg trace time vs cube length for cube* files."""
    # Filter only cube files and extract sizes
    cube_data = []
    for fname, stats_dict in files_stats.items():
        size = extract_cube_size(fname)
        if size is not None:
            cube_data.append((size, stats_dict["initial_phase_time"], stats_dict["average_trace_time"]))
    if not cube_data:
        print("No cube data found for scaling plot.")
        return
    cube_data.sort(key=lambda x: x[0])
    sizes = [d[0] for d in cube_data]
    phase1 = [d[1] for d in cube_data]
    avg_trace = [d[2] for d in cube_data]

    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    #fig.suptitle("Algorithm Scaling with Cube Size", fontsize=18)

    # Phase 1 time vs cube size
    ax = axes[0]
    ax.plot(sizes, phase1, marker="o", linewidth=2, markersize=8, color="#55A868")
    ax.set_xlabel("Cube length")
    ax.set_ylabel("Phase 1 time (s)")
    ax.set_title("Phase 1 Time vs Cube Size")
    ax.grid(True, alpha=0.3)

    # Avg trace time vs cube size
    ax = axes[1]
    ax.plot(sizes, avg_trace, marker="s", linewidth=2, markersize=8, color="#C44E52")
    ax.set_xlabel("Cube length")
    ax.set_ylabel("Average trace time (s)")
    ax.set_title("Average Trace Time vs Cube Size")
    ax.grid(True, alpha=0.3)

    fig.tight_layout(rect=[0,0,1,0.96])
    if save:
        out_file = f"{out_prefix}.pdf"
        fig.savefig(out_file, format='pdf', bbox_inches='tight')
        print(f"Saved scaling figure: {out_file}")
        plt.close(fig)
    else:
        plt.show()


def plot_scaling_messages(files_stats, save=False, out_prefix="scaling_messages"):
    """Plot phase 1 messages and avg trace messages vs cube length for cube* files."""
    # Filter only cube files and extract sizes
    cube_data = []
    for fname, stats_dict in files_stats.items():
        size = extract_cube_size(fname)
        if size is not None:
            cube_data.append((size, stats_dict["initial_phase_messages"], stats_dict["average_trace_messages"]))
    if not cube_data:
        print("No cube data found for message scaling plot.")
        return
    cube_data.sort(key=lambda x: x[0])
    sizes = [d[0] for d in cube_data]
    phase1_msgs = [d[1] for d in cube_data]
    avg_trace_msgs = [d[2] for d in cube_data]

    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    #fig.suptitle("Message Count Scaling with Cube Size", fontsize=18)

    # Phase 1 messages vs cube size
    ax = axes[0]
    ax.plot(sizes, phase1_msgs, marker="o", linewidth=2, markersize=8, color="#55A868")
    ax.set_xlabel("Cube length")
    ax.set_ylabel("Phase 1 messages")
    ax.set_title("Phase 1 Messages vs Cube Size")
    ax.grid(True, alpha=0.3)

    # Avg trace messages vs cube size
    ax = axes[1]
    ax.plot(sizes, avg_trace_msgs, marker="s", linewidth=2, markersize=8, color="#C44E52")
    ax.set_xlabel("Cube length")
    ax.set_ylabel("Average trace messages")
    ax.set_title("Average Trace Messages vs Cube Size")
    ax.grid(True, alpha=0.3)

    fig.tight_layout(rect=[0,0,1,0.96])
    if save:
        out_file = f"{out_prefix}.pdf"
        fig.savefig(out_file, format='pdf', bbox_inches='tight')
        print(f"Saved message scaling figure: {out_file}")
        plt.close(fig)
    else:
        plt.show()


def plot_config_comparison(files_stats, save=False, out_prefix="config_comparison"):
    """Plot comparison of config3d, configFilled, and configHoles with 4 bar plots."""
    # Filter config files (not cubes)
    config_names = ["config3d.xml", "configFilled.xml", "configHoles.xml"]
    config_data = {}
    
    for fname, stats_dict in files_stats.items():
        for cname in config_names:
            if cname in fname:
                # Extract display name
                display_name = cname.replace(".xml", "").replace("config", "")
                if display_name == "3d":
                    display_name = "3D"
                elif display_name == "Filled":
                    display_name = "Filled"
                elif display_name == "Holes":
                    display_name = "Holes"
                config_data[display_name] = stats_dict
                break
    
    if not config_data:
        print("No config files found for comparison plot.")
        return
    
    # Sort by display name for consistent ordering
    sorted_configs = sorted(config_data.items())
    names = [c[0] for c in sorted_configs]
    phase1_times = [c[1]["initial_phase_time"] for c in sorted_configs]
    phase1_msgs = [c[1]["initial_phase_messages"] for c in sorted_configs]
    avg_trace_times = [c[1]["average_trace_time"] for c in sorted_configs]
    avg_trace_msgs = [c[1]["average_trace_messages"] for c in sorted_configs]
    
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    
    # Phase 1 time
    ax = axes[0][0]
    ax.bar(names, phase1_times, color="#55A868")
    ax.set_ylabel("Time (s)")
    ax.set_title("Phase 1 Time")
    ax.grid(True, alpha=0.3, axis='y')
    
    # Phase 1 messages
    ax = axes[0][1]
    ax.bar(names, phase1_msgs, color="#4C72B0")
    ax.set_ylabel("Messages")
    ax.set_title("Phase 1 Messages")
    ax.grid(True, alpha=0.3, axis='y')
    
    # Avg trace time
    ax = axes[1][0]
    ax.bar(names, avg_trace_times, color="#C44E52")
    ax.set_ylabel("Time (s)")
    ax.set_title("Average Trace Time")
    ax.grid(True, alpha=0.3, axis='y')
    
    # Avg trace messages
    ax = axes[1][1]
    ax.bar(names, avg_trace_msgs, color="#DD8452")
    ax.set_ylabel("Messages")
    ax.set_title("Average Trace Messages")
    ax.grid(True, alpha=0.3, axis='y')
    
    fig.tight_layout()
    if save:
        out_file = f"{out_prefix}.pdf"
        fig.savefig(out_file, format='pdf', bbox_inches='tight')
        print(f"Saved config comparison figure: {out_file}")
        plt.close(fig)
    else:
        plt.show()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dir", type=Path, default=Path(__file__).parent, help="Directory containing aina26_results_*.json")
    parser.add_argument("--save", action="store_true", help="Save figures instead of showing (default: show)")
    args = parser.parse_args()

    json_files = sorted(args.dir.glob("aina26_results_*.json"))
    if not json_files:
        print("No JSON result files found in", args.dir)
        return

    files_stats = {}
    for jf in json_files:
        try:
            data = load_json(jf)
        except Exception as e:
            print(f"Skipping {jf}: {e}")
            continue
        if not all(k in data for k in ("recordedTimes", "recordedNbMessages")):
            print(f"Skipping {jf}: missing required keys")
            continue
        stats_dict = compute_trace_stats(data["recordedTimes"])
        msg_stats = compute_message_stats(data["recordedNbMessages"])
        # Merge both dicts
        stats_dict.update(msg_stats)
        files_stats[jf.name] = stats_dict
        # Minimal summary line
        print(f"{jf.name}: phase1={stats_dict['initial_phase_time']:.2f}s msgs={stats_dict['initial_phase_messages']} avgTrace={stats_dict['average_trace_time']:.2f}s avgMsgs={stats_dict['average_trace_messages']:.0f}")

    # Always generate the three summary PDFs
    plot_scaling(files_stats, save=args.save, out_prefix="scaling")
    plot_scaling_messages(files_stats, save=args.save, out_prefix="scaling_messages")
    plot_config_comparison(files_stats, save=args.save, out_prefix="config_comparison")

if __name__ == "__main__":
    main()
