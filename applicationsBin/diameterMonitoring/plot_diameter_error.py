#!/usr/bin/env python3
"""
Plot error analysis between measured and real diameter values.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import re
from pathlib import Path

# Create output folder for plots
output_folder = Path("plots")
output_folder.mkdir(exist_ok=True)
csv_file = "simulation_results_random_diameter_check.csv"
df = pd.read_csv(csv_file)

# Calculate error metrics
df['absolute_error'] = np.abs(df['diameter'] - df['real_diameter'])
df['signed_error'] = df['diameter'] - df['real_diameter']

# Extract num_robots and occupancy from config_file using regex
def extract_parameters(config_file):
    match = re.match(r'random_(\d+)_(\d+)_\d+\.xml', config_file)
    if match:
        return int(match.group(1)), int(match.group(2))
    return None, None

df[['num_robots_config', 'occupancy_config']] = df['config_file'].apply(
    lambda x: pd.Series(extract_parameters(x))
)

# Aggregate by num_robots and occupancy (mean across 10 trials)
agg_df = df.groupby(['num_robots_config', 'occupancy_config']).agg({
    'absolute_error': ['mean', 'std', 'min', 'max'],
    'signed_error': ['mean', 'std'],
    'diameter': 'mean',
    'real_diameter': 'mean',
    'num_messages': 'mean',
}).reset_index()

agg_df.columns = ['num_robots', 'occupancy', 'abs_error_mean', 'abs_error_std', 
                  'abs_error_min', 'abs_error_max', 'signed_error_mean', 'signed_error_std',
                  'diameter_mean', 'real_diameter_mean', 'num_messages_mean']

# Create output folder for plots
output_folder = Path("plots")
output_folder.mkdir(exist_ok=True)

# Read the CSV file
csv_file = "simulation_results_random_diameter_check.csv"
df = pd.read_csv(csv_file)

# Calculate error metrics
df['absolute_error'] = np.abs(df['diameter'] - df['real_diameter'])
df['signed_error'] = df['diameter'] - df['real_diameter']

# Extract num_robots and occupancy from config_file using regex
def extract_parameters(config_file):
    match = re.match(r'random_(\d+)_(\d+)_\d+\.xml', config_file)
    if match:
        return int(match.group(1)), int(match.group(2))
    return None, None

df[['num_robots_config', 'occupancy_config']] = df['config_file'].apply(
    lambda x: pd.Series(extract_parameters(x))
)

# Aggregate by num_robots and occupancy (mean across 10 trials)
agg_df = df.groupby(['num_robots_config', 'occupancy_config']).agg({
    'absolute_error': ['mean', 'std', 'min', 'max'],
    'signed_error': ['mean', 'std'],
    'diameter': 'mean',
    'real_diameter': 'mean',
    'num_messages': 'mean',
}).reset_index()

agg_df.columns = ['num_robots', 'occupancy', 'abs_error_mean', 'abs_error_std', 
                  'abs_error_min', 'abs_error_max', 'signed_error_mean', 'signed_error_std',
                  'diameter_mean', 'real_diameter_mean', 'num_messages_mean']

robot_counts = sorted(agg_df['num_robots'].unique())
occupancy_levels = sorted(agg_df['occupancy'].unique())

# Plot 1: Heatmap
fig1, ax1 = plt.subplots(figsize=(10, 8))
pivot_error = agg_df.pivot(index='num_robots', columns='occupancy', values='abs_error_mean')
im1 = ax1.imshow(pivot_error, cmap='YlOrRd', aspect='auto')
ax1.set_xticks(range(len(pivot_error.columns)))
ax1.set_yticks(range(len(pivot_error.index)))
ax1.set_xticklabels(pivot_error.columns)
ax1.set_yticklabels(pivot_error.index)
ax1.set_xlabel('Occupancy Level (%)', fontsize=12)
ax1.set_ylabel('Number of Robots', fontsize=12)
ax1.set_title('Mean Absolute Error Heatmap', fontsize=14, fontweight='bold')
for i in range(len(pivot_error.index)):
    for j in range(len(pivot_error.columns)):
        value = pivot_error.iloc[i, j]
        if not np.isnan(value):
            ax1.text(j, i, f'{value:.2f}', ha='center', va='center', 
                    color='white' if value > pivot_error.max().max() * 0.5 else 'black', fontsize=10)
cbar1 = plt.colorbar(im1, ax=ax1, label='Mean Absolute Error')
plt.tight_layout()
plt.savefig(output_folder / '1_heatmap_error.pdf', format='pdf', dpi=300, bbox_inches='tight')
print(f"Saved: {output_folder / '1_heatmap_error.pdf'}")
plt.close()

# Plot 2: Error vs occupancy
fig2, ax2 = plt.subplots(figsize=(11, 7))
colors = plt.cm.Set3(np.linspace(0, 1, len(robot_counts)))
for i, num_robots in enumerate(robot_counts):
    subset = agg_df[agg_df['num_robots'] == num_robots].sort_values('occupancy')
    ax2.errorbar(subset['occupancy'], subset['abs_error_mean'], 
                xerr=0, yerr=subset['abs_error_std'],
                marker='o', linestyle='-', linewidth=2.5, markersize=8, 
                label=f'{int(num_robots)} robots', color=colors[i], capsize=6, capthick=2)
ax2.set_xlabel('Occupancy Level (%)', fontsize=12)
ax2.set_ylabel('Mean Absolute Error', fontsize=12)
ax2.set_title('Error vs Occupancy Level (with ±1 Std Dev)', fontsize=14, fontweight='bold')
ax2.legend(loc='best', fontsize=11)
ax2.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig(output_folder / '2_error_vs_occupancy.pdf', format='pdf', dpi=300, bbox_inches='tight')
print(f"Saved: {output_folder / '2_error_vs_occupancy.pdf'}")
plt.close()

# Plot 3: Error vs number of robots
fig3, ax3 = plt.subplots(figsize=(11, 7))
colors = plt.cm.Set2(np.linspace(0, 1, len(occupancy_levels)))
for i, occupancy in enumerate(occupancy_levels):
    subset = agg_df[agg_df['occupancy'] == occupancy].sort_values('num_robots')
    ax3.errorbar(subset['num_robots'], subset['abs_error_mean'],
                xerr=0, yerr=subset['abs_error_std'],
                marker='s', linestyle='-', linewidth=2.5, markersize=8,
                label=f'{int(occupancy)}% occupancy', color=colors[i], capsize=6, capthick=2)
ax3.set_xlabel('Number of Robots', fontsize=12)
ax3.set_ylabel('Mean Absolute Error', fontsize=12)
ax3.set_title('Error vs Number of Robots (with ±1 Std Dev)', fontsize=14, fontweight='bold')
ax3.legend(loc='best', fontsize=11)
ax3.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig(output_folder / '3_error_vs_robotcount.pdf', format='pdf', dpi=300, bbox_inches='tight')
print(f"Saved: {output_folder / '3_error_vs_robotcount.pdf'}")
plt.close()

# Plot 4: Bar plot comparison
fig4, ax4 = plt.subplots(figsize=(12, 7))
x_pos = np.arange(len(robot_counts))
width = 0.2
for i, occupancy in enumerate(occupancy_levels):
    subset = agg_df[agg_df['occupancy'] == occupancy].sort_values('num_robots')
    values = [agg_df[(agg_df['num_robots'] == rb) & (agg_df['occupancy'] == occupancy)]['abs_error_mean'].values[0] 
              if len(agg_df[(agg_df['num_robots'] == rb) & (agg_df['occupancy'] == occupancy)]) > 0 else 0 
              for rb in robot_counts]
    ax4.bar(x_pos + i * width, values, width, label=f'{int(occupancy)}% occupancy', color=colors[i])
ax4.set_xlabel('Number of Robots', fontsize=12)
ax4.set_ylabel('Mean Absolute Error', fontsize=12)
ax4.set_title('Error Comparison by Occupancy at Each Scale', fontsize=14, fontweight='bold')
ax4.set_xticks(x_pos + width * 1.5)
ax4.set_xticklabels([int(rb) for rb in robot_counts])
ax4.legend(fontsize=11)
ax4.grid(True, alpha=0.3, axis='y')
plt.tight_layout()
plt.savefig(output_folder / '4_occupancy_comparison.pdf', format='pdf', dpi=300, bbox_inches='tight')
print(f"Saved: {output_folder / '4_occupancy_comparison.pdf'}")
plt.close()

# Plot 5: Histogram
fig5, ax5 = plt.subplots(figsize=(11, 7))
ax5.hist(df['absolute_error'], bins=20, color='coral', edgecolor='black', alpha=0.7)
ax5.axvline(df['absolute_error'].mean(), color='red', linestyle='--', linewidth=2.5, label=f'Mean: {df["absolute_error"].mean():.3f}')
ax5.axvline(df['absolute_error'].median(), color='green', linestyle='--', linewidth=2.5, label=f'Median: {df["absolute_error"].median():.3f}')
ax5.set_xlabel('Absolute Error', fontsize=12)
ax5.set_ylabel('Frequency (# of trials)', fontsize=12)
ax5.set_title('Distribution of Absolute Error (All Trials)', fontsize=14, fontweight='bold')
ax5.legend(fontsize=11)
ax5.grid(True, alpha=0.3, axis='y')
plt.tight_layout()
plt.savefig(output_folder / '5_error_distribution.pdf', format='pdf', dpi=300, bbox_inches='tight')
print(f"Saved: {output_folder / '5_error_distribution.pdf'}")
plt.close()

# Plot 6: Scatter
fig6, ax6 = plt.subplots(figsize=(10, 8))
ax6.scatter(df['real_diameter'], df['diameter'], alpha=0.5, s=40, color='steelblue')
min_val = min(df['real_diameter'].min(), df['diameter'].min())
max_val = max(df['real_diameter'].max(), df['diameter'].max())
ax6.plot([min_val, max_val], [min_val, max_val], 'r--', label='Perfect (y=x)', linewidth=2.5)
ax6.set_xlabel('Real Diameter', fontsize=12)
ax6.set_ylabel('Measured Diameter', fontsize=12)
ax6.set_title('Measured vs Real Diameter (All Trials)', fontsize=14, fontweight='bold')
ax6.legend(fontsize=11)
ax6.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig(output_folder / '6_measured_vs_real.pdf', format='pdf', dpi=300, bbox_inches='tight')
print(f"Saved: {output_folder / '6_measured_vs_real.pdf'}")
plt.close()

print(f"\nAll plots saved to '{output_folder}/' folder")

# Print summary statistics
print("\n" + "="*60)
print("DIAMETER ERROR ANALYSIS SUMMARY (Aggregated by Size & Occupancy)")
print("="*60)
print(f"Total unique configurations: {len(agg_df)}")
print(f"Configuration matrix: {len(robot_counts)} robot counts × {len(occupancy_levels)} occupancy levels")
print(f"  Robot counts: {robot_counts}")
print(f"  Occupancy levels: {occupancy_levels}")
print(f"Total trials analyzed: {len(df)} (10 per configuration)")

print(f"\nOverall Absolute Error Statistics (across all configurations):")
print(f"  Mean:     {agg_df['abs_error_mean'].mean():.4f}")
print(f"  Median:   {agg_df['abs_error_mean'].median():.4f}")
print(f"  Std Dev:  {agg_df['abs_error_mean'].std():.4f}")
print(f"  Min:      {agg_df['abs_error_mean'].min():.4f}")
print(f"  Max:      {agg_df['abs_error_mean'].max():.4f}")

print(f"\nError by Number of Robots (averaged across occupancy levels):")
for num_robots in robot_counts:
    subset = agg_df[agg_df['num_robots'] == num_robots]
    print(f"  {num_robots:3d} robots: Mean Error = {subset['abs_error_mean'].mean():.4f} (±{subset['abs_error_std'].mean():.4f})")

print(f"\nError by Occupancy Level (averaged across robot counts):")
for occ in occupancy_levels:
    subset = agg_df[agg_df['occupancy'] == occ]
    print(f"  {occ:2d}% occupancy: Mean Error = {subset['abs_error_mean'].mean():.4f} (±{subset['abs_error_std'].mean():.4f})")

print(f"\nDetailed Configuration Results (Mean ± Std across 10 trials):")
print(f"{'Robots':<8} {'Occupancy':<12} {'Abs Error':<15} {'Signed Error':<15}")
print("-" * 50)
for _, row in agg_df.sort_values(['num_robots', 'occupancy']).iterrows():
    print(f"{int(row['num_robots']):<8} {int(row['occupancy']):>3d}%{'':<8} {row['abs_error_mean']:.4f} ± {row['abs_error_std']:.4f}{'':<3} {row['signed_error_mean']:>6.4f} ± {row['signed_error_std']:.4f}")

print(f"\nPerfect Predictions Analysis (across all {len(df)} trials):")
perfect_count = (df['absolute_error'] == 0).sum()
print(f"  Count: {perfect_count}")
print(f"  Percentage: {perfect_count / len(df) * 100:.2f}%")
