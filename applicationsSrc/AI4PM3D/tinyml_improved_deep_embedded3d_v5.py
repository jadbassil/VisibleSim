"""
tinyml_improved_deep_embedded3d_v5.py
======================================
Improved Deep Embedded 3D Distributed CNN — 6-Class Model (v5.0)

Classes:
  0: Cube
  1: Sphere
  2: Cylinder
  3: Chair
  4: Bridge
  5: Humanoid (Lego Guy shape with variable joint angles)
"""

import numpy as np
import time
import scipy.ndimage
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers

print(f"TensorFlow version: {tf.__version__}")
print(f"GPU available: {len(tf.config.list_physical_devices('GPU')) > 0}")

# =============================================================================
# HYPERPARAMETERS
# =============================================================================

GRID        = 16      # Voxel grid size
K           = 32      # Feature channels per voxel
K_FC1       = 48      # Hidden layer size in the 2-layer FC head
NUM_CLASSES = 6       # 0=Cube, 1=Sphere, 2=Cylinder, 3=Chair, 4=Bridge, 5=Humanoid

CLASS_NAMES = ['Cube', 'Sphere', 'Cylinder', 'Chair', 'Bridge', 'Humanoid']

NUM_TRAIN   = 60000   # Training samples (10k per class × 6 classes)
NUM_VAL     = 6000    # Validation samples
NUM_TEST    = 3000    # Test samples for final eval
BATCH_SIZE  = 128
EPOCHS      = 200     # Epochs
WARMUP_EPOCHS = 10
LR_PEAK     = 1e-3
LR_END      = 1e-6
LABEL_SMOOTH = 0.1    # Label smoothing factor

OUTPUT_HEADER = "neuralNetworkData_deep_embedded3d.h"

# =============================================================================
# Shape Generators
# =============================================================================

def add_noise(grid, rng, noise_rate=0.01):
    noise = rng.rand(*grid.shape) < noise_rate
    return np.clip(grid + noise, 0, 1).astype(np.float32)

def generate_cube(G, rng):
    size = rng.randint(3, min(G - 2, 13))
    x0 = rng.randint(1, max(2, G - size))
    y0 = rng.randint(1, max(2, G - size))
    z0 = rng.randint(1, max(2, G - size))
    grid = np.zeros((G, G, G), dtype=np.float32)
    grid[x0:x0+size, y0:y0+size, z0:z0+size] = 1.0
    if rng.rand() < 0.1:
        sx = max(3, int(size * rng.uniform(0.8, 1.25)))
        sy = max(3, int(size * rng.uniform(0.8, 1.25)))
        sz = max(3, int(size * rng.uniform(0.8, 1.25)))
        sx, sy, sz = min(sx, G-2), min(sy, G-2), min(sz, G-2)
        grid = np.zeros((G, G, G), dtype=np.float32)
        grid[x0:x0+sx, y0:y0+sy, z0:z0+sz] = 1.0
    return grid

def generate_rotated_cube(G, rng):
    size = rng.randint(4, min(G - 4, 10))
    grid = np.zeros((G, G, G), dtype=np.float32)
    center = G // 2
    half = size // 2
    grid[center-half:center+size-half,
         center-half:center+size-half,
         center-half:center+size-half] = 1.0

    ax, ay, az = rng.uniform(0, 360, 3)
    rotated = scipy.ndimage.rotate(grid, ax, axes=(1, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, ay, axes=(0, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, az, axes=(0, 1), reshape=False, order=0)

    max_shift = max(0, G // 2 - size - 1)
    if max_shift > 0:
        shift = rng.randint(-max_shift, max_shift + 1, 3)
        rotated = scipy.ndimage.shift(rotated, shift, order=0)

    return (rotated > 0.5).astype(np.float32)

def generate_small_cube(G, rng):
    size = rng.randint(2, 5)
    x0   = rng.randint(1, max(2, G - size))
    y0   = rng.randint(1, max(2, G - size))
    z0   = rng.randint(1, max(2, G - size))
    grid = np.zeros((G, G, G), dtype=np.float32)
    grid[x0:x0+size, y0:y0+size, z0:z0+size] = 1.0
    return grid

def generate_overflowing_cube(G, rng):
    size = rng.randint(12, 19)
    x0 = rng.randint(-5, 5)
    y0 = rng.randint(-5, 5)
    z0 = rng.randint(-5, 5)
    grid = np.zeros((G, G, G), dtype=np.float32)
    x_min, x_max = max(0, x0), min(G, x0 + size)
    y_min, y_max = max(0, y0), min(G, y0 + size)
    z_min, z_max = max(0, z0), min(G, z0 + size)
    grid[x_min:x_max, y_min:y_max, z_min:z_max] = 1.0
    return grid

def generate_overflowing_rotated_cube(G, rng):
    size = rng.randint(10, 16)
    grid = np.zeros((G, G, G), dtype=np.float32)
    c = G // 2
    h = size // 2
    grid[c-h:c+size-h, c-h:c+size-h, c-h:c+size-h] = 1.0
    ax, ay, az = rng.uniform(0, 360, 3)
    rotated = scipy.ndimage.rotate(grid, ax, axes=(1, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, ay, axes=(0, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, az, axes=(0, 1), reshape=False, order=0)
    shift = rng.randint(-6, 7, 3)
    rotated = scipy.ndimage.shift(rotated, shift, order=0)
    return (rotated > 0.5).astype(np.float32)

def generate_sphere(G, rng):
    radius = rng.uniform(1.5, min(G // 2 - 0.5, 7.5))
    margin = radius + 1.0
    cx = rng.uniform(margin, G - margin)
    cy = rng.uniform(margin, G - margin)
    cz = rng.uniform(margin, G - margin)
    coords = np.mgrid[0:G, 0:G, 0:G].astype(np.float32)
    if rng.rand() < 0.4:
        sx = rng.uniform(0.6, 2.0)
        sy = rng.uniform(0.6, 2.0)
        sz = rng.uniform(0.6, 2.0)
        dist = np.sqrt(((coords[0] - cx)/sx)**2 + ((coords[1] - cy)/sy)**2 + ((coords[2] - cz)/sz)**2)
    else:
        dist = np.sqrt((coords[0] - cx)**2 + (coords[1] - cy)**2 + (coords[2] - cz)**2)
    return (dist <= radius).astype(np.float32)

def generate_small_sphere(G, rng):
    radius = rng.uniform(1.0, 2.5)
    margin = max(radius + 1.0, 2.0)
    cx = rng.uniform(margin, G - margin)
    cy = rng.uniform(margin, G - margin)
    cz = rng.uniform(margin, G - margin)
    coords = np.mgrid[0:G, 0:G, 0:G].astype(np.float32)
    dist = np.sqrt((coords[0] - cx)**2 + (coords[1] - cy)**2 + (coords[2] - cz)**2)
    return (dist <= radius).astype(np.float32)

def generate_overflowing_sphere(G, rng):
    radius = rng.uniform(7.0, 13.0)
    cx = rng.uniform(-4, G + 4)
    cy = rng.uniform(-4, G + 4)
    cz = rng.uniform(-4, G + 4)
    coords = np.mgrid[0:G, 0:G, 0:G].astype(np.float32)
    dist = np.sqrt((coords[0] - cx)**2 + (coords[1] - cy)**2 + (coords[2] - cz)**2)
    return (dist <= radius).astype(np.float32)

def generate_cylinder(G, rng):
    radius = rng.uniform(1.5, min(G // 2 - 2, 6.0))
    length = rng.uniform(3.0, min(G - 3, 13.0))
    axis = rng.randint(3)
    margin = max(radius + 1, length / 2 + 1)
    center = [rng.uniform(margin, G - margin) for _ in range(3)]
    coords = np.mgrid[0:G, 0:G, 0:G].astype(np.float32)
    perp_axes = [i for i in range(3) if i != axis]
    perp_dist = np.sqrt((coords[perp_axes[0]] - center[perp_axes[0]])**2 + (coords[perp_axes[1]] - center[perp_axes[1]])**2)
    along_dist = np.abs(coords[axis] - center[axis])
    if rng.rand() < 0.1:
        scale = rng.uniform(0.8, 1.25)
        pa = perp_axes[0]
        perp_dist = np.sqrt(((coords[pa] - center[pa])/scale)**2 + (coords[perp_axes[1]] - center[perp_axes[1]])**2)
    return ((perp_dist <= radius) & (along_dist <= length / 2)).astype(np.float32)

def generate_rotated_cylinder(G, rng):
    grid = generate_cylinder(G, rng)
    ax, ay, az = rng.uniform(0, 360, 3)
    rotated = scipy.ndimage.rotate(grid, ax, axes=(1, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, ay, axes=(0, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, az, axes=(0, 1), reshape=False, order=0)
    return (rotated > 0.5).astype(np.float32)

def generate_overflowing_cylinder(G, rng):
    radius = rng.uniform(5.0, 10.0)
    length = rng.uniform(10.0, 25.0)
    axis = rng.randint(3)
    center = [rng.uniform(-4, G + 4) for _ in range(3)]
    coords = np.mgrid[0:G, 0:G, 0:G].astype(np.float32)
    perp_axes = [i for i in range(3) if i != axis]
    perp_dist = np.sqrt((coords[perp_axes[0]] - center[perp_axes[0]])**2 + (coords[perp_axes[1]] - center[perp_axes[1]])**2)
    along_dist = np.abs(coords[axis] - center[axis])
    return ((perp_dist <= radius) & (along_dist <= length / 2)).astype(np.float32)

def generate_overflowing_rotated_cylinder(G, rng):
    grid = generate_overflowing_cylinder(G, rng)
    ax, ay, az = rng.uniform(0, 360, 3)
    rotated = scipy.ndimage.rotate(grid, ax, axes=(1, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, ay, axes=(0, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, az, axes=(0, 1), reshape=False, order=0)
    return (rotated > 0.5).astype(np.float32)

def generate_chair(G, rng):
    seat_w = rng.uniform(3.0, min(G - 4, 7.0))
    seat_d = rng.uniform(3.0, min(G - 4, 7.0))
    seat_t = rng.uniform(0.6, 1.5)
    leg_t = rng.uniform(0.5, 1.2)
    leg_h = rng.uniform(2.0, min(G // 2 - 1, 6.0))
    back_t = rng.uniform(0.5, 1.2)
    back_h = rng.uniform(2.0, min(G // 2 - 2, 5.0))
    total_h = leg_h + seat_t + back_h
    up_axis = rng.randint(3)
    side_axes = [i for i in range(3) if i != up_axis]
    ax_w, ax_d = side_axes[0], side_axes[1]
    margin = max(seat_w / 2, seat_d / 2, total_h / 2) + 1.5
    margin = min(margin, G / 2 - 0.5)
    center = [rng.uniform(margin, G - margin) for _ in range(3)]
    coords = np.mgrid[0:G, 0:G, 0:G].astype(np.float32)
    grid = np.zeros((G, G, G), dtype=np.float32)
    base = center[up_axis] - total_h / 2

    # 4 Legs
    leg_offsets_w = [center[ax_w] - seat_w / 2 + leg_t / 2, center[ax_w] + seat_w / 2 - leg_t / 2]
    leg_offsets_d = [center[ax_d] - seat_d / 2 + leg_t / 2, center[ax_d] + seat_d / 2 - leg_t / 2]
    for lw in leg_offsets_w:
        for ld in leg_offsets_d:
            w_mask = np.abs(coords[ax_w] - lw) <= leg_t / 2
            d_mask = np.abs(coords[ax_d] - ld) <= leg_t / 2
            u_mask = (coords[up_axis] >= base) & (coords[up_axis] <= base + leg_h)
            grid = np.maximum(grid, (w_mask & d_mask & u_mask).astype(np.float32))

    # Seat
    seat_bottom = base + leg_h
    seat_center_u = seat_bottom + seat_t / 2
    w_mask = np.abs(coords[ax_w] - center[ax_w]) <= seat_w / 2
    d_mask = np.abs(coords[ax_d] - center[ax_d]) <= seat_d / 2
    u_mask = np.abs(coords[up_axis] - seat_center_u) <= seat_t / 2
    grid = np.maximum(grid, (w_mask & d_mask & u_mask).astype(np.float32))

    # Backrest
    back_w_center = center[ax_w] - seat_w / 2 + back_t / 2
    back_u_center = seat_bottom + seat_t + back_h / 2
    w_mask = np.abs(coords[ax_w] - back_w_center) <= back_t / 2
    d_mask = np.abs(coords[ax_d] - center[ax_d]) <= seat_d / 2
    u_mask = np.abs(coords[up_axis] - back_u_center) <= back_h / 2
    grid = np.maximum(grid, (w_mask & d_mask & u_mask).astype(np.float32))
    return grid

def generate_rotated_chair(G, rng):
    grid = generate_chair(G, rng)
    ax, ay, az = rng.uniform(0, 360, 3)
    rotated = scipy.ndimage.rotate(grid, ax, axes=(1, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, ay, axes=(0, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, az, axes=(0, 1), reshape=False, order=0)
    return (rotated > 0.5).astype(np.float32)

def generate_overflowing_chair(G, rng):
    scale = rng.uniform(1.5, 3.5)
    seat_w = rng.uniform(3.0, 7.0) * scale
    seat_d = rng.uniform(3.0, 7.0) * scale
    seat_t = rng.uniform(0.6, 1.5) * scale
    leg_t  = rng.uniform(0.5, 1.2) * scale
    leg_h  = rng.uniform(2.0, 6.0) * scale
    back_t = rng.uniform(0.5, 1.2) * scale
    back_h = rng.uniform(2.0, 5.0) * scale
    total_h = leg_h + seat_t + back_h
    up_axis = rng.randint(3)
    side_axes = [i for i in range(3) if i != up_axis]
    ax_w, ax_d = side_axes[0], side_axes[1]
    center = [rng.uniform(G * 0.15, G * 0.85) for _ in range(3)]
    coords = np.mgrid[0:G, 0:G, 0:G].astype(np.float32)
    grid = np.zeros((G, G, G), dtype=np.float32)
    base = center[up_axis] - total_h / 2

    # 4 Legs
    leg_offsets_w = [center[ax_w] - seat_w / 2 + leg_t / 2, center[ax_w] + seat_w / 2 - leg_t / 2]
    leg_offsets_d = [center[ax_d] - seat_d / 2 + leg_t / 2, center[ax_d] + seat_d / 2 - leg_t / 2]
    for lw in leg_offsets_w:
        for ld in leg_offsets_d:
            w_mask = np.abs(coords[ax_w] - lw) <= leg_t / 2
            d_mask = np.abs(coords[ax_d] - ld) <= leg_t / 2
            u_mask = (coords[up_axis] >= base) & (coords[up_axis] <= base + leg_h)
            grid = np.maximum(grid, (w_mask & d_mask & u_mask).astype(np.float32))

    # Seat
    seat_bottom = base + leg_h
    seat_center_u = seat_bottom + seat_t / 2
    w_mask = np.abs(coords[ax_w] - center[ax_w]) <= seat_w / 2
    d_mask = np.abs(coords[ax_d] - center[ax_d]) <= seat_d / 2
    u_mask = np.abs(coords[up_axis] - seat_center_u) <= seat_t / 2
    grid = np.maximum(grid, (w_mask & d_mask & u_mask).astype(np.float32))

    # Backrest
    back_w_center = center[ax_w] - seat_w / 2 + back_t / 2
    back_u_center = seat_bottom + seat_t + back_h / 2
    w_mask = np.abs(coords[ax_w] - back_w_center) <= back_t / 2
    d_mask = np.abs(coords[ax_d] - center[ax_d]) <= seat_d / 2
    u_mask = np.abs(coords[up_axis] - back_u_center) <= back_h / 2
    grid = np.maximum(grid, (w_mask & d_mask & u_mask).astype(np.float32))
    return grid

def generate_bridge(G, rng):
    deck_len  = rng.uniform(4.0, min(G - 3, 11.0))
    deck_w    = rng.uniform(1.5, min(G - 3, 5.0))
    deck_t    = rng.uniform(0.5, 1.2)
    pillar_tl = rng.uniform(0.5, 1.2)
    pillar_tw = deck_w * rng.uniform(0.9, 1.0)
    pillar_h  = rng.uniform(2.0, min(G // 2 - 1, 6.0))
    total_h = pillar_h + deck_t
    up_axis = rng.randint(3)
    remaining = [i for i in range(3) if i != up_axis]
    len_axis, w_axis = remaining[0], remaining[1]
    margin = max(deck_len / 2, deck_w / 2, total_h / 2) + 1.5
    margin = min(margin, G / 2 - 0.5)
    center = [rng.uniform(margin, G - margin) for _ in range(3)]
    coords = np.mgrid[0:G, 0:G, 0:G].astype(np.float32)
    grid = np.zeros((G, G, G), dtype=np.float32)
    base = center[up_axis] - total_h / 2

    # 2 Pillars
    pillar_positions = [center[len_axis] - deck_len / 2 + pillar_tl / 2, center[len_axis] + deck_len / 2 - pillar_tl / 2]
    for pl in pillar_positions:
        l_mask = np.abs(coords[len_axis] - pl) <= pillar_tl / 2
        w_mask = np.abs(coords[w_axis] - center[w_axis]) <= pillar_tw / 2
        u_mask = (coords[up_axis] >= base) & (coords[up_axis] <= base + pillar_h)
        grid = np.maximum(grid, (l_mask & w_mask & u_mask).astype(np.float32))

    # Deck
    deck_center_u = base + pillar_h + deck_t / 2
    l_mask = np.abs(coords[len_axis] - center[len_axis]) <= deck_len / 2
    w_mask = np.abs(coords[w_axis] - center[w_axis]) <= deck_w / 2
    u_mask = np.abs(coords[up_axis] - deck_center_u) <= deck_t / 2
    grid = np.maximum(grid, (l_mask & w_mask & u_mask).astype(np.float32))
    return grid

def generate_rotated_bridge(G, rng):
    grid = generate_bridge(G, rng)
    ax, ay, az = rng.uniform(0, 360, 3)
    rotated = scipy.ndimage.rotate(grid, ax, axes=(1, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, ay, axes=(0, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, az, axes=(0, 1), reshape=False, order=0)
    return (rotated > 0.5).astype(np.float32)

def generate_overflowing_bridge(G, rng):
    scale = rng.uniform(1.5, 3.5)
    deck_len  = rng.uniform(5.0, 12.0) * scale
    deck_w    = rng.uniform(2.0, 5.0) * scale
    deck_t    = rng.uniform(0.5, 1.5) * scale
    pillar_tl = rng.uniform(0.5, 1.5) * scale
    pillar_tw = rng.uniform(1.0, 3.0) * scale
    pillar_h  = rng.uniform(3.0, 8.0) * scale
    total_h = pillar_h + deck_t
    up_axis = rng.randint(3)
    remaining = [i for i in range(3) if i != up_axis]
    len_axis, w_axis = remaining[0], remaining[1]
    center = [rng.uniform(G * 0.15, G * 0.85) for _ in range(3)]
    coords = np.mgrid[0:G, 0:G, 0:G].astype(np.float32)
    grid = np.zeros((G, G, G), dtype=np.float32)
    base = center[up_axis] - total_h / 2

    # 2 Pillars
    pillar_positions = [center[len_axis] - deck_len / 2 + pillar_tl / 2, center[len_axis] + deck_len / 2 - pillar_tl / 2]
    for pl in pillar_positions:
        l_mask = np.abs(coords[len_axis] - pl) <= pillar_tl / 2
        w_mask = np.abs(coords[w_axis] - center[w_axis]) <= pillar_tw / 2
        u_mask = (coords[up_axis] >= base) & (coords[up_axis] <= base + pillar_h)
        grid = np.maximum(grid, (l_mask & w_mask & u_mask).astype(np.float32))

    # Deck
    deck_center_u = base + pillar_h + deck_t / 2
    l_mask = np.abs(coords[len_axis] - center[len_axis]) <= deck_len / 2
    w_mask = np.abs(coords[w_axis] - center[w_axis]) <= deck_w / 2
    u_mask = np.abs(coords[up_axis] - deck_center_u) <= deck_t / 2
    grid = np.maximum(grid, (l_mask & w_mask & u_mask).astype(np.float32))
    return grid

# =============================================================================
# HUMAN GENERATOR (Rigid Body posturing for Lego Guy shape)
# =============================================================================

def generate_humanoid(G, rng):
    """
    Generate a procedurally articulated humanoid shape.
    It attaches torso, head, neck, arms, and legs.
    Random postures swing legs and arms.
    """
    grid = np.zeros((G, G, G), dtype=np.float32)
    coords = np.mgrid[0:G, 0:G, 0:G].astype(np.float32)
    X, Y, Z = coords[0], coords[1], coords[2]

    # Torso Center
    cx, cy, cz = G/2, G/2, G/2

    # Torso dimensions
    torso_h = rng.uniform(4.0, 5.5)
    torso_w = rng.uniform(2.5, 3.5)
    torso_d = rng.uniform(1.5, 2.2)

    # Torso mask
    torso_mask = (np.abs(X - cx) <= torso_w/2) & (np.abs(Y - cy) <= torso_d/2) & (np.abs(Z - cz) <= torso_h/2)
    grid = np.maximum(grid, torso_mask.astype(np.float32))

    # Head
    head_r = rng.uniform(1.3, 1.8)
    hcz = cz + torso_h/2 + head_r * 0.8
    head_dist = np.sqrt((X - cx)**2 + (Y - cy)**2 + (Z - hcz)**2)
    head_mask = head_dist <= head_r
    grid = np.maximum(grid, head_mask.astype(np.float32))

    # Neck
    neck_h = 1.0
    neck_r = 0.8
    neck_cz = cz + torso_h/2 + neck_h/2
    neck_mask = ((X - cx)**2 + (Y - cy)**2 <= neck_r**2) & (np.abs(Z - neck_cz) <= neck_h/2)
    grid = np.maximum(grid, neck_mask.astype(np.float32))

    def add_limb(joint_x, joint_y, joint_z, limb_r, limb_len, theta_deg, phi_deg):
        t = np.radians(theta_deg)
        p = np.radians(phi_deg)
        dx = X - joint_x
        dy = Y - joint_y
        dz = Z - joint_z
        # Y rotation
        dx1 = dx * np.cos(t) - dz * np.sin(t)
        dz1 = dx * np.sin(t) + dz * np.cos(t)
        dy1 = dy
        # X rotation
        dy2 = dy1 * np.cos(p) - dz1 * np.sin(p)
        dz2 = dy1 * np.sin(p) + dz1 * np.cos(p)
        dx2 = dx1
        dist_sq = dx2**2 + dy2**2
        mask = (dist_sq <= limb_r**2) & (dz2 <= 0) & (dz2 >= -limb_len)
        return mask.astype(np.float32)

    # Legs (hinge swing forward/back & out/in)
    leg_len = rng.uniform(3.5, 5.0)
    leg_r = rng.uniform(0.7, 1.0)
    leg_offset = torso_w * 0.35

    leg_l_theta = rng.uniform(-15, 15)
    leg_l_phi = rng.uniform(-30, 45) if rng.rand() < 0.45 else rng.uniform(-10, 10) # swing
    leg_r_theta = rng.uniform(-15, 15)
    leg_r_phi = rng.uniform(-30, 45) if rng.rand() < 0.45 else rng.uniform(-10, 10)

    grid = np.maximum(grid, add_limb(cx - leg_offset, cy, cz - torso_h/2, leg_r, leg_len, leg_l_theta, leg_l_phi))
    grid = np.maximum(grid, add_limb(cx + leg_offset, cy, cz - torso_h/2, leg_r, leg_len, leg_r_theta, leg_r_phi))

    # Arms (broad spherical joint angles)
    arm_len = rng.uniform(3.5, 5.0)
    arm_r = rng.uniform(0.6, 0.9)
    arm_offset = torso_w/2 + arm_r * 0.5

    arm_l_theta = rng.uniform(30, 180) if rng.rand() < 0.5 else rng.uniform(-45, 45) # raise/swivel
    arm_l_phi = rng.uniform(-45, 90)
    arm_r_theta = rng.uniform(-180, -30) if rng.rand() < 0.5 else rng.uniform(-45, 45)
    arm_r_phi = rng.uniform(-45, 90)

    grid = np.maximum(grid, add_limb(cx - arm_offset, cy, cz + torso_h/2 - arm_r, arm_r, arm_len, arm_l_theta, arm_l_phi))
    grid = np.maximum(grid, add_limb(cx + arm_offset, cy, cz + torso_h/2 - arm_r, arm_r, arm_len, arm_r_theta, arm_r_phi))

    return grid

def generate_rotated_humanoid(G, rng):
    grid = generate_humanoid(G, rng)
    ax, ay, az = rng.uniform(0, 360, 3)
    rotated = scipy.ndimage.rotate(grid, ax, axes=(1, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, ay, axes=(0, 2), reshape=False, order=0)
    rotated = scipy.ndimage.rotate(rotated, az, axes=(0, 1), reshape=False, order=0)
    return (rotated > 0.5).astype(np.float32)

def generate_overflowing_humanoid(G, rng):
    grid = generate_humanoid(G, rng)
    scale = rng.uniform(1.3, 2.5)
    zoomed = scipy.ndimage.zoom(grid, scale, order=1)
    # Crop or pad to GxGxGx
    s = zoomed.shape[0]
    if s > G:
        start = (s - G) // 2
        grid = zoomed[start:start+G, start:start+G, start:start+G]
    else:
        pad = (G - s) // 2
        grid = np.pad(zoomed, ((pad, G - s - pad), (pad, G - s - pad), (pad, G - s - pad)), mode='constant')
    return (grid > 0.5).astype(np.float32)

# =============================================================================
# DATA PIPELINE
# =============================================================================

def data_generator(num_samples, grid_size=GRID, seed=None, is_validation=False):
    rng  = np.random.RandomState(seed if seed is not None else np.random.randint(1 << 31))
    X    = np.zeros((num_samples, grid_size, grid_size, grid_size, 1), dtype=np.float32)
    y    = np.zeros((num_samples, NUM_CLASSES), dtype=np.float32)
    G    = grid_size
    G3   = G ** 3

    for i in range(num_samples):
        cls  = i % NUM_CLASSES  # balanced labels
        attempts = 0

        while True:
            attempts += 1
            roll = rng.rand()

            if cls == 0:  # Cube
                if roll < 0.20: shape = generate_cube(G, rng)
                elif roll < 0.45: shape = generate_rotated_cube(G, rng)
                elif roll < 0.70: shape = generate_overflowing_cube(G, rng)
                else: shape = generate_small_cube(G, rng)
            elif cls == 1:  # Sphere
                if roll < 0.40: shape = generate_sphere(G, rng)
                elif roll < 0.80: shape = generate_overflowing_sphere(G, rng)
                else: shape = generate_small_sphere(G, rng)
            elif cls == 2:  # Cylinder
                if roll < 0.25: shape = generate_cylinder(G, rng)
                elif roll < 0.50: shape = generate_rotated_cylinder(G, rng)
                elif roll < 0.75: shape = generate_overflowing_cylinder(G, rng)
                else: shape = generate_overflowing_rotated_cylinder(G, rng)
            elif cls == 3:  # Chair
                if roll < 0.35: shape = generate_chair(G, rng)
                elif roll < 0.70: shape = generate_rotated_chair(G, rng)
                else: shape = generate_overflowing_chair(G, rng)
            elif cls == 4:  # Bridge
                if roll < 0.35: shape = generate_bridge(G, rng)
                elif roll < 0.70: shape = generate_rotated_bridge(G, rng)
                else: shape = generate_overflowing_bridge(G, rng)
            else:  # cls == 5  Humanoid (Lego Guy)
                if roll < 0.35: shape = generate_humanoid(G, rng)
                elif roll < 0.70: shape = generate_rotated_humanoid(G, rng)
                else: shape = generate_overflowing_humanoid(G, rng)

            occ = shape.sum()
            if occ >= 6 and occ <= G3 * 0.97:
                break
            if attempts > 15:
                shape = generate_cube(G, rng)
                break

        # Chirality augmentation: random independent axis flips make the model
        # invariant to left-handed hardware coordinate systems.
        # Rotation alone cannot fix chirality — a mirror image of a Chair or
        # Bridge is still a Chair/Bridge but the old model had never seen it.
        if rng.rand() < 0.5: shape = np.ascontiguousarray(shape[::-1, :, :])
        if rng.rand() < 0.5: shape = np.ascontiguousarray(shape[:, ::-1, :])
        if rng.rand() < 0.5: shape = np.ascontiguousarray(shape[:, :, ::-1])

        if not is_validation:
            shape = add_noise(shape, rng, noise_rate=0.01)

        X[i, ..., 0] = shape
        label = [0.0] * NUM_CLASSES
        label[cls] = 1.0
        y[i] = label

    return X, y

# =============================================================================
# MODEL ARCHITECTURE
# =============================================================================

class FaceGatherLayer(layers.Layer):
    def __init__(self, **kwargs):
        super(FaceGatherLayer, self).__init__(**kwargs)

    def call(self, inputs):
        x, mask = inputs
        # Pad to shift
        x_pad = keras.ops.pad(x, [[0,0], [1,1], [1,1], [1,1], [0,0]], mode="constant")
        # Shifts
        x_xm = x_pad[:, 0:-2, 1:-1, 1:-1, :]
        x_xp = x_pad[:, 2:  , 1:-1, 1:-1, :]
        x_ym = x_pad[:, 1:-1, 0:-2, 1:-1, :]
        x_yp = x_pad[:, 1:-1, 2:  , 1:-1, :]
        x_zm = x_pad[:, 1:-1, 1:-1, 0:-2, :]
        x_zp = x_pad[:, 1:-1, 1:-1, 2:  , :]

        # Concatenate: [own, xm, xp, ym, yp, zm, zp]
        out = keras.ops.concatenate([x, x_xm, x_xp, x_ym, x_yp, x_zm, x_zp], axis=-1)
        # Apply mask
        out = out * keras.ops.cast(mask, out.dtype)
        return out

    def get_config(self):
        return super(FaceGatherLayer, self).get_config()

def build_model(grid_size=GRID, k=K, num_classes=NUM_CLASSES, k_fc1=K_FC1):
    input_voxels = keras.Input(shape=(grid_size, grid_size, grid_size, 1), name="voxels")
    mask = keras.ops.cast(input_voxels > 0.5, "float32")

    # Phase 1: Local Conv1
    c1 = layers.Conv3D(k, 3, padding="same", activation="relu", name="conv1_3d")(input_voxels)
    c1 = c1 * mask

    # Phase 2: FaceGather + Conv2 (Dense)
    gathered1 = FaceGatherLayer(name="face_gather1")([c1, mask])
    c2 = layers.Dense(k, activation="relu", name="conv2_face")(gathered1)
    c2 = c2 * mask
    c2_res = c2 + c1  # Residual Connection

    # Phase 3: FaceGather + Conv3 (Dense)
    gathered2 = FaceGatherLayer(name="face_gather2")([c2_res, mask])
    c3 = layers.Dense(k, activation="relu", name="conv3_face")(gathered2)
    c3 = c3 * mask
    c3_res = c3 + c2_res  # Residual Connection

    # Phase 4: Masked Global Average Pooling
    sum_feats = keras.ops.sum(c3_res, axis=[1, 2, 3])
    count = keras.ops.sum(mask, axis=[1, 2, 3])
    avg = sum_feats / keras.ops.maximum(count, 1.0)

    # Phase 5: Root Classification Head
    fc1 = layers.Dense(k_fc1, activation="relu", name="fc1_deep")(avg)
    dropout = layers.Dropout(0.3)(fc1)
    outputs = layers.Dense(num_classes, activation="softmax", name="fc2_deep")(dropout)

    return keras.Model(inputs=input_voxels, outputs=outputs, name="DeepEmbedded3D_CNN_v5")

# =============================================================================
# Learning Rate Schedule
# =============================================================================

class WarmupCosineDecay(keras.optimizers.schedules.LearningRateSchedule):
    def __init__(self, lr_peak, lr_end, warmup_steps, total_steps):
        super().__init__()
        self.lr_peak      = float(lr_peak)
        self.lr_end       = float(lr_end)
        self.warmup_steps = float(warmup_steps)
        self.total_steps  = float(total_steps)

    def __call__(self, step):
        step  = tf.cast(step, tf.float32)
        warmup_lr = self.lr_peak * (step / self.warmup_steps)
        decay_steps = self.total_steps - self.warmup_steps
        progress    = tf.minimum((step - self.warmup_steps) / decay_steps, 1.0)
        cos_decay   = 0.5 * (1.0 + tf.cos(np.pi * progress))
        cosine_lr   = self.lr_end + (self.lr_peak - self.lr_end) * cos_decay
        return tf.where(step < self.warmup_steps, warmup_lr, cosine_lr)

    def get_config(self):
        return dict(lr_peak=self.lr_peak, lr_end=self.lr_end,
                    warmup_steps=self.warmup_steps, total_steps=self.total_steps)

# =============================================================================
# C Header Export
# =============================================================================

def export_c_header(model, output_path):
    """Export trained weights directly to C header file without BN (since model has no BN layers)."""
    c1_w, c1_b = model.get_layer('conv1_3d').get_weights()
    d2_w, d2_b = model.get_layer('conv2_face').get_weights()
    d3_w, d3_b = model.get_layer('conv3_face').get_weights()
    fc1_w, fc1_b = model.get_layer('fc1_deep').get_weights()
    fc2_w, fc2_b = model.get_layer('fc2_deep').get_weights()

    # Transpose conv1 weight from [3,3,3,1,K] to [K][1][3][3][3] to match C++
    c1_w_c = c1_w.transpose(4, 3, 0, 1, 2)

    k    = int(c1_b.shape[0])
    kfc1 = int(fc1_b.shape[0])
    nc   = int(fc2_b.shape[0])

    assert d2_w.shape == (7 * k, k), f"conv2 shape {d2_w.shape} != ({7*k},{k})"
    assert d3_w.shape == (7 * k, k), f"conv3 shape {d3_w.shape} != ({7*k},{k})"
    assert fc1_w.shape == (k, kfc1),  f"fc1 shape {fc1_w.shape} != ({k},{kfc1})"
    assert fc2_w.shape == (kfc1, nc), f"fc2 shape {fc2_w.shape} != ({kfc1},{nc})"

    def arr1d(vals):
        return ','.join(f'{v:.8f}f' for v in vals)

    class_comment = " / ".join(CLASS_NAMES)

    with open(output_path, 'w') as f:
        f.write("// Auto-generated by tinyml_improved_deep_embedded3d_v5.py\n")
        f.write("// Deep Embedded 3D CNN v5 — 6-Class Model\n")
        f.write("// Classes: Cube / Sphere / Cylinder / Chair / Bridge / Humanoid\n")
        f.write("// C++ changes needed: Update class names array and color switch\n")
        f.write("#ifndef NEURALNETWORKDATA_DEEP_EMBEDDED3D_H\n")
        f.write("#define NEURALNETWORKDATA_DEEP_EMBEDDED3D_H\n\n")

        f.write(f"static const int DEEP_K         = {k};      // Feature channels\n")
        f.write(f"static const int DEEP_K_FC1      = {kfc1};     // FC hidden layer size\n")
        f.write(f"static const int DEEP_NUM_CLASSES = {nc};     // {class_comment}\n")
        f.write(f"static const int DEEP_COMBINED   = {7*k};    // 7*K = self + 6 face neighbors\n\n")

        f.write(f"// Phase 1: Conv3D kernel. Shape: [K][1][3][3][3]\n")
        f.write(f"static const float conv1_3d_kernel[{k}][1][3][3][3] = {{\n")
        for filt in range(k):
            f.write(f"  {{ // filter {filt}\n    {{\n")
            for i in range(3):
                f.write("      {\n")
                for j in range(3):
                    vals = ','.join(f'{c1_w_c[filt][0][i][j][kk]:.8f}f' for kk in range(3))
                    f.write(f"        {{{vals}}},\n")
                f.write("      },\n")
            f.write("    }\n  },\n")
        f.write("};\n\n")

        f.write(f"static const float conv1_3d_bias[{k}] = {{{arr1d(c1_b)}}};\n\n")

        f.write(f"// Phase 2: FaceGather Dense weights. Shape: [7K][K]\n")
        f.write(f"static const float conv2_face_weight[{7*k}][{k}] = {{\n")
        for i in range(7 * k):
            f.write(f"  {{{arr1d(d2_w[i])}}},\n")
        f.write("};\n\n")
        f.write(f"static const float conv2_face_bias[{k}] = {{{arr1d(d2_b)}}};\n\n")

        f.write(f"// Phase 3: FaceGather Dense weights. Shape: [7K][K]\n")
        f.write(f"static const float conv3_face_weight[{7*k}][{k}] = {{\n")
        for i in range(7 * k):
            f.write(f"  {{{arr1d(d3_w[i])}}},\n")
        f.write("};\n\n")
        f.write(f"static const float conv3_face_bias[{k}] = {{{arr1d(d3_b)}}};\n\n")

        f.write(f"// Phase 5a: FC hidden layer weights. Shape: [K][K_FC1]\n")
        f.write(f"static const float fc1_deep_weight[{k}][{kfc1}] = {{\n")
        for i in range(k):
            f.write(f"  {{{arr1d(fc1_w[i])}}},\n")
        f.write("};\n\n")
        f.write(f"static const float fc1_deep_bias[{kfc1}] = {{{arr1d(fc1_b)}}};\n\n")

        f.write(f"// Phase 5b: FC output layer weights. Shape: [K_FC1][{nc}]\n")
        f.write(f"static const float fc2_deep_weight[{kfc1}][{nc}] = {{\n")
        for i in range(kfc1):
            f.write(f"  {{{arr1d(fc2_w[i])}}},\n")
        f.write("};\n\n")
        f.write(f"static const float fc2_deep_bias[{nc}] = {{{arr1d(fc2_b)}}};\n\n")

        f.write("#endif // NEURALNETWORKDATA_DEEP_EMBEDDED3D_H\n")

    total_params = (c1_w.size + c1_b.size +
                    d2_w.size + d2_b.size +
                    d3_w.size + d3_b.size +
                    fc1_w.size + fc1_b.size +
                    fc2_w.size + fc2_b.size)

    print(f"\n  Exported to: {output_path}")
    print(f"  DEEP_K={k}, DEEP_K_FC1={kfc1}, 7K={7*k}, NUM_CLASSES={nc}")
    print(f"  Total parameters: {total_params:,d} ({total_params * 4:,d} bytes = "
          f"{total_params * 4 / 1024:.1f} KB)")

# =============================================================================
# TRAINING AND TFLITE CONVERSION EXPORT
# =============================================================================

def train_and_export():
    print("=" * 65)
    print("  DEEP EMBEDDED 3D CNN  —  v5.0 (6-Class Model)")
    print("=" * 65)
    print(f"  K={K}, K_FC1={K_FC1}, Grid={GRID}³, Classes={NUM_CLASSES}")
    print(f"  Classes: {', '.join(CLASS_NAMES)}")
    print(f"  Architecture: Conv3D → [FaceGather+Dense] × 2 → AvgPool → 2×FC")
    print(f"  Procedural humanoid postures dataset enabled.")
    print("=" * 65)

    model = build_model()
    model.summary()

    steps_per_epoch = NUM_TRAIN // BATCH_SIZE
    total_steps     = steps_per_epoch * EPOCHS
    warmup_steps    = steps_per_epoch * WARMUP_EPOCHS

    lr_schedule = WarmupCosineDecay(
        lr_peak      = LR_PEAK,
        lr_end       = LR_END,
        warmup_steps = warmup_steps,
        total_steps  = total_steps
    )

    optimizer = keras.optimizers.Adam(
        learning_rate = lr_schedule,
        clipnorm      = 1.0
    )

    model.compile(
        optimizer = optimizer,
        loss      = 'categorical_crossentropy',
        metrics   = ['accuracy']
    )

    # ── Generate training data ──
    print(f"\n[2/5] Generating {NUM_TRAIN:,d} training shapes (6 classes)...")
    t0      = time.time()
    X_train, y_train = data_generator(NUM_TRAIN, seed=None, is_validation=False)
    X_val,   y_val   = data_generator(NUM_VAL,   seed=99,   is_validation=True)
    print(f"  Done in {time.time()-t0:.1f}s")

    # Check class balance
    print(f"\n  Training class balance:")
    for ci, name in enumerate(CLASS_NAMES):
        count = int((y_train[:, ci] > 0.5).sum())
        print(f"    {name:9s}: {count:6d}  ({count/NUM_TRAIN*100:.1f}%)")

    # ── Train ──
    print(f"\n[3/5] Training for up to {EPOCHS} epochs...")
    model.fit(
        X_train, y_train,
        batch_size       = BATCH_SIZE,
        validation_data  = (X_val, y_val),
        epochs           = EPOCHS,
        callbacks        = [
            keras.callbacks.EarlyStopping(
                monitor             = 'val_accuracy',
                patience            = 30,
                mode                = 'max',
                restore_best_weights= True,
                verbose             = 1
            ),
        ],
        verbose = 1
    )

    # ── Evaluate ──
    print(f"\n[4/5] Evaluating on {NUM_TEST:,d} held-out test shapes...")
    X_test, y_test = data_generator(NUM_TEST, seed=7777, is_validation=True)
    test_loss, test_acc = model.evaluate(X_test, y_test, verbose=0)
    print(f"\n  Test loss:     {test_loss:.4f}")
    print(f"  Test accuracy: {test_acc*100:.1f}%")

    preds  = model.predict(X_test, verbose=0)
    true_c = y_test.argmax(axis=1)
    pred_c = preds.argmax(axis=1)

    print(f"\n  Per-class accuracy:")
    for ci, name in enumerate(CLASS_NAMES):
        mask = (true_c == ci)
        if mask.sum() > 0:
            acc = (pred_c[mask] == ci).mean() * 100
            print(f"    {name:9s}: {(pred_c[mask]==ci).sum()}/{mask.sum()}  ({acc:.1f}%)")

    # ── Export C header ──
    print(f"\n[5/5] Exporting weights to C header...")
    export_c_header(model, OUTPUT_HEADER)

    print("\n" + "=" * 65)
    print("  DONE — v5.0 (6-Class Model)")
    print("=" * 65)

if __name__ == "__main__":
    train_and_export()
