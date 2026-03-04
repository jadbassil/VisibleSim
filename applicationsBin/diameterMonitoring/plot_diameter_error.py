#!/usr/bin/env python3
"""
Plot error analysis between measured and real diameter values.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# Read the CSV file
csv_file = "simulation_results_random_diameter_check.csv"
df = pd.read_csv(csv_file)

# Calculate error metrics
df['absolute_error'] = np.abs(df['diameter'] - df['real_diameter'])
df['signed_error'] = df['diameter'] - df['real_diameter']

# Create figure with multiple subplots
fig, axes = plt.subplots(2, 2, figsize=(14, 10))
fig.suptitle('Diameter Measurement Error Analysis', fontsize=16, fontweight='bold')

# 1. Scatter plot: Real diameter vs Measured diameter
ax1 = axes[0, 0]
ax1.scatter(df['real_diameter'], df['diameter'], alpha=0.6, s=50, color='steelblue')
min_val = min(df['real_diameter'].min(), df['diameter'].min())
max_val = max(df['real_diameter'].max(), df['diameter'].max())
ax1.plot([min_val, max_val], [min_val, max_val], 'r--', label='Perfect (y=x)', linewidth=2)
ax1.set_xlabel('Real Diameter', fontsize=11)
ax1.set_ylabel('Measured Diameter', fontsize=11)
ax1.set_title('Measured vs Real Diameter')
ax1.legend()
ax1.grid(True, alpha=0.3)

# 2. Histogram of absolute error
ax2 = axes[0, 1]
ax2.hist(df['absolute_error'], bins=15, color='coral', edgecolor='black', alpha=0.7)
ax2.axvline(df['absolute_error'].mean(), color='red', linestyle='--', linewidth=2, label=f'Mean: {df["absolute_error"].mean():.2f}')
ax2.axvline(df['absolute_error'].median(), color='green', linestyle='--', linewidth=2, label=f'Median: {df["absolute_error"].median():.2f}')
ax2.set_xlabel('Absolute Error', fontsize=11)
ax2.set_ylabel('Frequency', fontsize=11)
ax2.set_title('Distribution of Absolute Error')
ax2.legend()
ax2.grid(True, alpha=0.3, axis='y')

# 3. Line plot: Error over trials
ax3 = axes[1, 0]
ax3.plot(df.index, df['absolute_error'], marker='o', linestyle='-', linewidth=1.5, markersize=4, color='steelblue', label='Absolute Error')
ax3.fill_between(df.index, 0, df['absolute_error'], alpha=0.3, color='steelblue')
ax3.axhline(df['absolute_error'].mean(), color='red', linestyle='--', linewidth=2, label=f'Mean: {df["absolute_error"].mean():.2f}')
ax3.set_xlabel('Trial Index', fontsize=11)
ax3.set_ylabel('Absolute Error', fontsize=11)
ax3.set_title('Absolute Error Over Trials')
ax3.legend()
ax3.grid(True, alpha=0.3)

# 4. Box plot by occupancy level
ax4 = axes[1, 1]
occupancy_levels = sorted(df['occupancy'].unique())
error_by_occupancy = [df[df['occupancy'] == occ]['absolute_error'].values for occ in occupancy_levels]
bp = ax4.boxplot(error_by_occupancy, labels=occupancy_levels, patch_artist=True)
for patch in bp['boxes']:
    patch.set_facecolor('lightblue')
ax4.set_xlabel('Occupancy Level (%)', fontsize=11)
ax4.set_ylabel('Absolute Error', fontsize=11)
ax4.set_title('Error Distribution by Occupancy Level')
ax4.grid(True, alpha=0.3, axis='y')

plt.tight_layout()
plt.savefig('diameter_error_analysis.png', dpi=300, bbox_inches='tight')
print("Plot saved as 'diameter_error_analysis.png'")

# Print summary statistics
print("\n" + "="*50)
print("DIAMETER ERROR ANALYSIS SUMMARY")
print("="*50)
print(f"Total trials: {len(df)}")
print(f"\nAbsolute Error Statistics:")
print(f"  Mean:     {df['absolute_error'].mean():.4f}")
print(f"  Median:   {df['absolute_error'].median():.4f}")
print(f"  Std Dev:  {df['absolute_error'].std():.4f}")
print(f"  Min:      {df['absolute_error'].min():.4f}")
print(f"  Max:      {df['absolute_error'].max():.4f}")
print(f"  Count:    {len(df['absolute_error'])}")

print(f"\nSigned Error Statistics:")
print(f"  Mean:     {df['signed_error'].mean():.4f}")
print(f"  Median:   {df['signed_error'].median():.4f}")
print(f"  Std Dev:  {df['signed_error'].std():.4f}")

print(f"\nError by Occupancy Level:")
for occ in occupancy_levels:
    occ_errors = df[df['occupancy'] == occ]['absolute_error']
    print(f"  Occupancy {occ:2d}%: Mean Error = {occ_errors.mean():.4f}, Median = {occ_errors.median():.4f}")

print(f"\nNumber of perfect predictions (error=0): {(df['absolute_error'] == 0).sum()}")
print(f"Percentage of perfect predictions: {(df['absolute_error'] == 0).sum() / len(df) * 100:.2f}%")

plt.show()
