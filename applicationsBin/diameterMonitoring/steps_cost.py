import matplotlib.pyplot as plt
import numpy as np

# Data
steps = [1, 2, 3, 4, 5, 6]
diameters = [18, 19, 19, 20, 20, 18]
messages = [2563, 4674, 4695, 5240, 5276, 7097]
times = [309590, 638940, 641911, 701346, 705846, 989497]
events = ['Initial', 'Removal', 'Stability', 'Growth', 'Stability', 'Bridge']

# Convert times from microseconds to milliseconds
times_ms = [t / 1000 for t in times]

# Create figure with two y-axes
fig, ax1 = plt.subplots(figsize=(7, 3.5))

# Plot messages
color1 = 'tab:blue'
ax1.set_xlabel('Step', fontsize=12)
ax1.set_ylabel('Number of Messages', fontsize=12)
line1 = ax1.plot(steps, messages, 'o-', color=color1, linewidth=2, markersize=8, label='Messages')
ax1.tick_params(axis='y')
ax1.grid(True, alpha=0.3)

# Create second y-axis for time
ax2 = ax1.twinx()
color2 = 'tab:orange'
ax2.set_ylabel('Time (ms)', fontsize=12)
line2 = ax2.plot(steps, times_ms, 's-', color=color2, linewidth=2, markersize=8, label='Time')
ax2.tick_params(axis='y')

# Add diameter as text labels in the legend
diameter_label = 'Diameter: ' + ', '.join([f'{d}' for d in diameters])

# Add event labels
for i, (step, event) in enumerate(zip(steps, events)):
    # latest step annotation
    if i == len(steps) - 1:
        ax1.annotate(event, (step, messages[i]), 
                    textcoords="offset points", xytext=(0,-40), 
                    ha='center', fontsize=10, rotation=45)
    else:
        ax1.annotate(event, (step, messages[i]), 
                    textcoords="offset points", xytext=(0,10), 
                    ha='center', fontsize=10, rotation=45)

# Combine legends
lines = line1 + line2
labels = [l.get_label() for l in lines] 
ax1.legend(lines + [plt.Line2D([0], [0], color='none')], labels, loc='upper left')

#plt.title('Diameter Monitoring Cost Analysis', fontsize=14, fontweight='bold')
plt.tight_layout()
plt.savefig('cost_analysis.pdf', dpi=300, bbox_inches='tight')
plt.show()