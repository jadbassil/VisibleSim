"""Plot the float32 vs int8 confusion matrices from quantization_confusion_matrices.csv.

Usage: python plot_quantization_confusion.py
Writes latex/figures/AI4PM-CNN/AI4PM-DeepEmbedded/quantization_confusion.pdf (and .png).
"""
import csv
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.colors import LinearSegmentedColormap

HERE = Path(__file__).resolve().parent
CSV_PATH = HERE / "quantization_confusion_matrices.csv"
OUT_DIR = HERE / "latex" / "figures" / "AI4PM-CNN" / "AI4PM-DeepEmbedded"
OUT_STEM = "quantization_confusion"

# Single-hue sequential ramp (light -> dark blue), starting from near-white so
# empty cells recede into the page.
CMAP = LinearSegmentedColormap.from_list(
    "seq_blue", ["#ffffff", "#cde2fb", "#86b6ef", "#3987e5", "#1c5cab", "#0d366b"])
INK = "#1f1f1f"
MUTED = "#6b6b6b"


def load(path):
    with open(path, newline="") as f:
        rows = list(csv.reader(f))
    classes = rows[0][2:]
    mats = {}
    for r in rows[1:]:
        mats.setdefault(r[0], []).append([int(v) for v in r[2:]])
    return classes, {k: np.array(v) for k, v in mats.items()}


def main():
    classes, mats = load(CSV_PATH)
    plt.rcParams.update({"font.family": "serif", "font.size": 9,
                         "axes.edgecolor": MUTED, "axes.linewidth": 0.6})
    fig, axes = plt.subplots(1, 2, figsize=(7.0, 3.35), constrained_layout=True)

    for ax, (label, title) in zip(axes, [("FP32", "Float32"), ("INT8", "Int8 (quantized)")]):
        cm = mats[label]
        rate = cm / cm.sum(axis=1, keepdims=True)        # row-normalized: share of each true class
        im = ax.imshow(rate, cmap=CMAP, vmin=0, vmax=1)
        acc = np.trace(cm) / cm.sum() * 100
        ax.set_title(f"{title}: {acc:.2f}% accuracy", color=INK, fontsize=10, pad=6)
        ax.set_xticks(range(len(classes)), classes, rotation=40, ha="right", color=INK)
        ax.set_yticks(range(len(classes)), classes, color=INK)
        ax.set_xlabel("Predicted class", color=INK)
        ax.tick_params(length=0)
        # 2px-style white gaps between cells
        ax.set_xticks(np.arange(-.5, len(classes)), minor=True)
        ax.set_yticks(np.arange(-.5, len(classes)), minor=True)
        ax.grid(which="minor", color="white", linewidth=1.5)
        ax.tick_params(which="minor", length=0)
        for s in ax.spines.values():
            s.set_visible(False)
        for i in range(cm.shape[0]):
            for j in range(cm.shape[1]):
                v = cm[i, j]
                if v == 0:
                    continue
                ax.text(j, i, str(v), ha="center", va="center", fontsize=8,
                        color="white" if rate[i, j] > 0.5 else INK,
                        fontweight="bold" if i == j else "normal")
    axes[0].set_ylabel("True class", color=INK)
    axes[1].set_yticklabels([])  # shared rows: labels only on the left panel

    cbar = fig.colorbar(im, ax=axes, shrink=0.85, pad=0.02)
    cbar.set_label("Share of true class", color=INK)
    cbar.outline.set_visible(False)
    cbar.ax.tick_params(length=0, colors=INK)

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for ext in ("pdf", "png"):
        # bbox_inches="tight" grows the canvas to fit titles/axis labels, which
        # constrained_layout alone clipped in the PDF backend.
        fig.savefig(OUT_DIR / f"{OUT_STEM}.{ext}", dpi=300, bbox_inches="tight", pad_inches=0.04)
    print(f"Wrote {OUT_DIR / OUT_STEM}.pdf/.png")


if __name__ == "__main__":
    main()
