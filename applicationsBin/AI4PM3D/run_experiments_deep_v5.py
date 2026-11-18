import subprocess
import re
import csv
import os
import sys
import tempfile
import math
import itertools
from xml.sax.saxutils import escape

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, '..', '..'))

# CMake puts the binary next to this script (applicationsBin/AI4PM3D).
EXE_NAME = 'AI4PM_DeepEmb3D.exe' if os.name == 'nt' else 'AI4PM_DeepEmb3D'
EXE_PATH = os.environ.get('AI4PM_EXE', os.path.join(SCRIPT_DIR, EXE_NAME))
BASE_CONFIG = os.path.join(SCRIPT_DIR, 'config_cube_csg.xml')   # template only, never modified
CSV_OUTPUT = os.path.join(PROJECT_ROOT, 'applicationsSrc', 'AI4PM3D', 'deep_cnn_experiment_results_v5.csv')
EXE_CWD = SCRIPT_DIR
RESULT_FILE = os.path.join(EXE_CWD, 'cnn_result.txt')  # written by the simulator itself
RUN_TIMEOUT_S = 600

ANSI_ESCAPE = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
CLASSES = ["Cube", "Sphere", "Cylinder", "Chair", "Bridge", "Humanoid"]


def write_config(csg_code, out_path, grid=50):
    """Build a run-specific config from the template (shape only, no base plate)."""
    with open(BASE_CONFIG, 'r') as f:
        content = f.read()

    csg_attr = escape(f"\n      union() {{\n        color([1,1,1]) {csg_code}\n      }}", {'"': '&quot;'})
    # Use a function replacement so backslashes in the CSG are never treated as regex escapes
    content = re.sub(r'content="[\s\S]*?"', lambda _m: f'content="{csg_attr}"', content, count=1)
    content = re.sub(r'gridSize="[^"]+"', f'gridSize="{grid},{grid},{grid}"', content)

    with open(out_path, 'w') as f:
        f.write(content)


def run_simulation(config_path):
    print("Running simulation...")

    if os.path.exists(RESULT_FILE):
        os.remove(RESULT_FILE)

    env = os.environ.copy()
    if os.name == 'nt':
        mingw = find_mingw_path()
        if mingw:
            env["PATH"] = mingw + os.pathsep + env.get("PATH", "")

    try:
        proc = subprocess.run(
            [EXE_PATH, "-c", config_path, "-t", "-R"],
            cwd=EXE_CWD, env=env, capture_output=True, text=True,
            errors='replace', timeout=RUN_TIMEOUT_S,
        )
        # The simulator can abort during teardown after printing results, so the
        # exit code is not used; the parsed output decides success.
        return proc.stdout + "\n" + proc.stderr
    except subprocess.TimeoutExpired as e:
        print(f"[WARNING] Simulation timed out after {RUN_TIMEOUT_S}s")
        out = e.stdout or ""
        return out.decode(errors='replace') if isinstance(out, bytes) else out


def find_mingw_path():
    base_dir = r"C:\Program Files\JetBrains"
    if os.path.exists(base_dir):
        for root, dirs, files in os.walk(base_dir):
            if root.endswith(r"bin\mingw\bin"):
                return root
    return None


def parse_output(output, actual_shape, description, csg_code):
    output = ANSI_ESCAPE.sub('', output)
    print(f"[DEBUG] Captured {len(output)} characters from simulator.")

    results = {
        "Actual Shape": actual_shape,
        "Description": description,
        "Predicted": "N/A",
        "Confidence": 0.0,
        "Blocks": 0,
        "Robots": 0,
        "Total Messages": 0,
        "Total Bandwidth (bytes)": 0,
        "Simulation Time (us)": 0,
        **{f"{c} %": 0.0 for c in CLASSES},
        "CSG Code": csg_code,
    }

    # Fall back to the result file the simulator writes if stdout lacks the summary
    text = output
    if not re.search(r"Predicted:", text) and os.path.exists(RESULT_FILE):
        with open(RESULT_FILE, "r") as rfile:
            text = rfile.read() + "\n" + output

    m = re.search(r"Predicted:\s+(\w+)\s+\(([\d.]+)%\)", text)
    if m:
        results["Predicted"] = m.group(1)
        results["Confidence"] = float(m.group(2))

    m = re.search(r"Blocks aggregated:\s+(\d+)", text)
    if m: results["Blocks"] = int(m.group(1))

    m = re.search(r"Number of robots:\s+(\d+)", text)
    if m: results["Robots"] = int(m.group(1))

    m = re.search(r"Total messages:\s+(\d+)", text)
    if m: results["Total Messages"] = int(m.group(1))

    m = re.search(r"Total bandwidth:\s+(\d+)", text)
    if m: results["Total Bandwidth (bytes)"] = int(m.group(1))

    # Simulated (not wall-clock) time from the global statistics block
    m = re.search(r"Simulator elapsed time:\s+(\d+)\s*us", text)
    if m: results["Simulation Time (us)"] = int(m.group(1))

    for c in CLASSES:
        m = re.search(rf"^\s*{c}:\s+([\d.]+)%", text, re.MULTILINE)
        if m: results[f"{c} %"] = float(m.group(1))

    return results

def get_humanoid_csg(scale_factors, trans, arm_l, arm_r, leg_l, leg_r):
    return f"""
wbody=15.4;
wshoulders=11.5;
hbody=13.2;
a=(wbody-wshoulders)/2;abody=atan(a/hbody)*180/3.14159265;
h =sqrt(a*a+hbody*hbody);
 
module head() {{
  union() {{
    cylinder(0.8,3.25,3.25,false);
    translate([0,0,0.8]) union() {{
      translate([0,0,1.5]) cylinder(5,5,5,false);
      cylinder(8,3.5,3.5,false);
      translate([0,0,1.5]) torus(3.5,1.5);
      translate([0,0,6.5]) torus(3.5,1.5);
    }}
    translate([0,0,8.8]) difference() {{
        cylinder(2,2.4,2.4,false);
        cylinder(2.1,1.6,1.6,false);
    }}
  }}
}}
 
module body() {{
    translate([0,-4,0])
    difference() {{
       union() {{
         translate([wbody/2,0,0]) rotate([0,-abody,0]) translate([-wbody/2,0,0]) cube([wbody/2,8,h]);
         translate([-wbody/2,0,0]) rotate([0,abody,0]) cube([wbody/2,8,h]);
         translate([-wshoulders/2,0,0]) cube([wshoulders,8,hbody]);
       }}
       translate([-8,-0.1,-3]) cube([16,8.2,3]);
   }}
}}
 
module Leg() {{
  union() {{
    cylinder(7.1,3.5,3.5,false);
    translate([0,-3.5,0]) cube([8.25,5.75,7.1]);
    translate([8.25,-3.5,0]) cube([3.2,7.2,7.1]);
  }}
}}
 
module Legs(al,ar) {{
    union() {{
        translate([-7.3,-4,-2.2]) cube([14.6,8,2.2]);
        translate([0,0,-4.2]) rotate([0,90,0]) union() {{
            cylinder(0.8,3.5,3.5,true);
            rotate([0,0,ar]) translate([0,0,0.3]) Leg();
            rotate([0,0,al]) translate([0,0,-7.4]) Leg();
        }}
    }}
}}
 
module Arms(al,ar) {{
    union() {{
        difference() {{
            translate([-wshoulders/2,0,hbody]) rotate([0,abody,0])
            translate([0,0,-3.2]) rotate([al,0,0]) rotate([0,180,0]) union() {{
                color([1,1,1]) sphere(2.75);
                rotate([0,13,0]) union() {{
                    color([1,1,1]) cylinder(5,2.75,2.75,false);
                    difference() {{
                        color([1,1,1]) translate([0.7,-1.35,3.6]) rotate([-45,0,0]) cylinder(6.95,2,2,false);
                        color([1,1,1]) difference() {{
                            cylinder(5,4,4,false);
                            cylinder(5,2.75,2.75,false);
                        }}
                    }}
                    color([1,1,0]) translate([0.7,-1.35,3.6]) rotate([-45,0,0]) translate([0.0,0.0,6.95]) rotate([0,0,-110]) union() {{
                        cylinder(1.9,1.2,1.2,false);
                        translate([0,0,3.5]) rotate([0,90,0]) difference() {{
                            cylinder(4.4,2.55,2.55,true);
                            cylinder(4.5,1.6,1.6,true);
                            translate([-3.3,0,0]) cube([4.5,5,4.5],true);
                        }}
                    }}
                }}
            }}
            translate([-wbody/2-0.01,-8.1,0]) rotate([0,abody,0]) cube([wbody/2,16,h]);
        }}
        difference() {{
            translate([wshoulders/2,0,hbody]) rotate([0,-abody,0])
            translate([0,0,-3.2]) rotate([ar,0,0]) rotate([0,180,0]) union() {{
                color([1,1,1]) sphere(2.75);
                rotate([0,-13,0]) union() {{
                    color([1,1,1]) cylinder(5,2.75,2.75,false);
                    color([1,1,1]) difference() {{
                        translate([-0.7,-1.35,3.6]) rotate([-45,0,0]) cylinder(6.95,2,2,false);
                        difference() {{
                            cylinder(5,4,4,false);
                            cylinder(5,2.75,2.75,false);
                        }}
                    }}
                    color([1,1,0]) translate([-0.7,-1.35,3.6]) rotate([-45,0,0]) translate([0.0,0.0,6.95]) rotate([0,0,-70]) union() {{
                        cylinder(1.9,1.2,1.2,false);
                        translate([0,0,3.5]) rotate([0,90,0]) difference() {{
                            cylinder(4.4,2.55,2.55,true);
                            cylinder(4.5,1.6,1.6,true);
                            translate([-3.3,0,0]) cube([4.5,5,4.5],true);
                        }}
                    }}
                }}
            }}
            translate([wbody/2+0.01,-20.1,0]) rotate([0,-abody,0]) translate([-wbody/2,16,0]) cube([wbody/2,16,h]);
        }}
    }}
}}
scale([{scale_factors[0]},{scale_factors[1]},{scale_factors[2]}]) translate([{trans[0]},{trans[1]},{trans[2]}]) union() {{
    color([1,0,0]) body();
    color([1,1,0]) translate([0,0,hbody]) head();
    color([0,0,1]) Legs({leg_l},{leg_r});
    Arms({arm_l},{arm_r});
}}
"""

# =============================================================================
# Training-matched shapes
# Mirror generate_humanoid / generate_bridge (and their rotated / overflowing
# variants) in applicationsSrc/AI4PM3D/tinyml_improved_deep_embedded3d_v5.py.
# Training voxelizes on a 16^3 grid and fills cell (i,j,k) when the point
# (i,j,k) is inside the shape; VisibleSim's CSG loader uses the same rule, so
# with the same parameters and a 16^3 world the block set matches training.
# =============================================================================

TRAIN_GRID = 16


def _n(v):
    """Compact number formatting for CSG."""
    return f"{v:.4f}".rstrip('0').rstrip('.') if isinstance(v, float) else str(v)


def _vec(v):
    return "[" + ",".join(_n(float(x)) for x in v) + "]"


def _humanoid_voxels(torso, head_r, legs, arms, G=TRAIN_GRID):
    """Pure-Python copy of generate_humanoid with explicit parameters (cells as (x,y,z))."""
    torso_h, torso_w, torso_d = torso
    cx = cy = cz = G / 2
    leg_len, leg_r, (llt, llp), (lrt, lrp) = legs
    arm_len, arm_r, (alt, alp), (art, arp) = arms
    leg_off, arm_off = torso_w * 0.35, torso_w/2 + arm_r*0.5
    limbs = [(cx - leg_off, cz - torso_h/2, leg_r, leg_len, llt, llp),
             (cx + leg_off, cz - torso_h/2, leg_r, leg_len, lrt, lrp),
             (cx - arm_off, cz + torso_h/2 - arm_r, arm_r, arm_len, alt, alp),
             (cx + arm_off, cz + torso_h/2 - arm_r, arm_r, arm_len, art, arp)]
    limbs = [(jx, jz, r, ln, math.cos(math.radians(t)), math.sin(math.radians(t)),
              math.cos(math.radians(p)), math.sin(math.radians(p))) for jx, jz, r, ln, t, p in limbs]
    hcz, ncz = cz + torso_h/2 + head_r*0.8, cz + torso_h/2 + 0.5
    vox = set()
    for x, y, z in itertools.product(range(G), repeat=3):
        inside = ((abs(x-cx) <= torso_w/2 and abs(y-cy) <= torso_d/2 and abs(z-cz) <= torso_h/2)
                  or math.sqrt((x-cx)**2 + (y-cy)**2 + (z-hcz)**2) <= head_r
                  or ((x-cx)**2 + (y-cy)**2 <= 0.8**2 and abs(z-ncz) <= 0.5))
        for jx, jz, r, ln, ct, st, cp, sp in limbs:
            if inside:
                break
            dx, dy, dz = x - jx, y - cy, z - jz
            dx1, dz1 = dx*ct - dz*st, dx*st + dz*ct
            dy2, dz2 = dy*cp - dz1*sp, dy*sp + dz1*cp
            inside = dx1*dx1 + dy2*dy2 <= r*r and -ln <= dz2 <= 0
        if inside:
            vox.add((x, y, z))
    return vox


def _bridge_voxels(deck_len, deck_w, deck_t, pillar_tl, pillar_tw, pillar_h, up_axis, center, G=TRAIN_GRID):
    """Pure-Python copy of generate_bridge / generate_overflowing_bridge with explicit parameters."""
    L, W = [i for i in range(3) if i != up_axis]
    base = center[up_axis] - (pillar_h + deck_t) / 2
    pillars = (center[L] - deck_len/2 + pillar_tl/2, center[L] + deck_len/2 - pillar_tl/2)
    deck_u = base + pillar_h + deck_t/2
    vox = set()
    for p in itertools.product(range(G), repeat=3):
        in_w = abs(p[W] - center[W])
        if ((any(abs(p[L] - pl) <= pillar_tl/2 for pl in pillars) and in_w <= pillar_tw/2
             and base <= p[up_axis] <= base + pillar_h)
                or (abs(p[L] - center[L]) <= deck_len/2 and in_w <= deck_w/2
                    and abs(p[up_axis] - deck_u) <= deck_t/2)):
            vox.add(p)
    return vox


def _rotate_voxels(vox, angles, G=TRAIN_GRID):
    """
    Same as the training generate_rotated_*: scipy.ndimage.rotate on axes (1,2), (0,2), (0,1)
    with reshape=False, order=0 (nearest), mode='constant'.
    """
    ctr = (G - 1) / 2

    def rot_plane(vox, a0, a1, deg):
        c, s = math.cos(math.radians(deg)), math.sin(math.radians(deg))
        # scipy: input = M @ output + offset, M = [[c, s], [-s, c]], offset = ctr - M @ ctr
        o0, o1 = ctr - (c + s)*ctr, ctr - (c - s)*ctr
        out = set()
        for p in itertools.product(range(G), repeat=3):
            n0 = math.floor(c*p[a0] + s*p[a1] + o0 + 0.5)
            n1 = math.floor(-s*p[a0] + c*p[a1] + o1 + 0.5)
            if 0 <= n0 < G and 0 <= n1 < G:
                q = list(p)
                q[a0], q[a1] = n0, n1
                if tuple(q) in vox:
                    out.add(p)
        return out

    ax, ay, az = angles
    return rot_plane(rot_plane(rot_plane(vox, 1, 2, ax), 0, 2, ay), 0, 1, az)


def _zoom_crop_voxels(vox, zoom, G=TRAIN_GRID):
    """
    Same as generate_overflowing_humanoid: scipy.ndimage.zoom(order=1), centre crop to G, > 0.5.
    zoom maps output index i to input i*(G-1)/(s-1) and interpolates linearly.
    """
    s_out = round(G * zoom)
    k = (G - 1) / (s_out - 1)
    start = (s_out - G) // 2

    def sample(c):
        lo = [min(math.floor(v), G - 2) for v in c]
        fr = [v - l for v, l in zip(c, lo)]
        acc = 0.0
        for d in itertools.product((0, 1), repeat=3):
            if (lo[0]+d[0], lo[1]+d[1], lo[2]+d[2]) in vox:
                w = 1.0
                for a in range(3):
                    w *= fr[a] if d[a] else 1 - fr[a]
                acc += w
        return acc

    return {p for p in itertools.product(range(G), repeat=3)
            if sample([(v + start) * k for v in p]) > 0.5}


def _voxels_csg(vox):
    """One small cube per cell: reproduces an exact voxel set in VisibleSim."""
    return "union() { " + " ".join(f"translate([{x},{y},{z}]) cube([0.5,0.5,0.5],true);"
                                   for x, y, z in sorted(vox)) + " }"


def humanoid_train_csg(torso, head_r, legs, arms, rotate=None, zoom=None, G=TRAIN_GRID):
    """
    torso = (h, w, d)                       training: h 4-5.5, w 2.5-3.5, d 1.5-2.2
    head_r                                  training: 1.3-1.8
    legs  = (len, r, (theta_l, phi_l), (theta_r, phi_r))   len 3.5-5, r 0.7-1.0
    arms  = (len, r, (theta_l, phi_l), (theta_r, phi_r))   len 3.5-5, r 0.6-0.9
    rotate = (ax, ay, az) -> generate_rotated_humanoid
    zoom   = 1.3-2.5      -> generate_overflowing_humanoid (zoom about corner, centre crop)
    """
    torso_h, torso_w, torso_d = torso
    cx = cy = cz = G / 2

    def limb(jx, jy, jz, r, length, theta, phi):
        # Training keeps points whose coords, rotated by Ry(theta) then Rx(phi), lie in a
        # cylinder along -z; the inverse placement is Rx(-phi) then Ry(theta).
        return (f"translate({_vec([jx, jy, jz])}) rotate({_vec([0, theta, 0])}) "
                f"rotate({_vec([-phi, 0, 0])}) translate({_vec([0, 0, -length])}) "
                f"cylinder({_n(float(length))},{_n(float(r))},{_n(float(r))},false);")

    parts = [
        f"translate({_vec([cx, cy, cz])}) cube({_vec([torso_w, torso_d, torso_h])},true);",
        f"translate({_vec([cx, cy, cz + torso_h/2 + head_r*0.8])}) sphere({_n(float(head_r))});",
        f"translate({_vec([cx, cy, cz + torso_h/2 + 0.5])}) cylinder(1,0.8,0.8,true);",
    ]
    leg_len, leg_r, (llt, llp), (lrt, lrp) = legs
    leg_off = torso_w * 0.35
    parts.append(limb(cx - leg_off, cy, cz - torso_h/2, leg_r, leg_len, llt, llp))
    parts.append(limb(cx + leg_off, cy, cz - torso_h/2, leg_r, leg_len, lrt, lrp))
    arm_len, arm_r, (alt, alp), (art, arp) = arms
    arm_off = torso_w/2 + arm_r*0.5
    parts.append(limb(cx - arm_off, cy, cz + torso_h/2 - arm_r, arm_r, arm_len, alt, alp))
    parts.append(limb(cx + arm_off, cy, cz + torso_h/2 - arm_r, arm_r, arm_len, art, arp))

    if zoom is None and rotate is None:
        return "union() { " + " ".join(parts) + " }"   # primitives give the exact training voxels
    # Rotated / overflowing variants resample the voxel array in training, so do the same here
    vox = _humanoid_voxels(torso, head_r, legs, arms, G)
    if zoom is not None:
        vox = _zoom_crop_voxels(vox, zoom, G)
    if rotate is not None:
        vox = _rotate_voxels(vox, rotate, G)
    return _voxels_csg(vox)


def bridge_train_csg(deck_len, deck_w, deck_t, pillar_tl, pillar_tw, pillar_h, up_axis, center,
                     rotate=None, G=TRAIN_GRID):
    """
    Two pillars + a deck, as in generate_bridge / generate_overflowing_bridge.
    training (standard): deck_len 4-11, deck_w 1.5-5, deck_t 0.5-1.2, pillar_tl 0.5-1.2,
                         pillar_tw = deck_w*0.9-1.0, pillar_h 2-6
    training (overflow): all of the above x scale 1.5-3.5, centre in [0.15G, 0.85G]
    rotate = (ax, ay, az) -> generate_rotated_bridge
    """
    L, W = [i for i in range(3) if i != up_axis]
    base = center[up_axis] - (pillar_h + deck_t) / 2

    def box(c_l, s_l, c_u, s_u, s_w):
        c, sz = [0.0]*3, [0.0]*3
        c[L], sz[L] = c_l, s_l
        c[W], sz[W] = center[W], s_w
        c[up_axis], sz[up_axis] = c_u, s_u
        return f"translate({_vec(c)}) cube({_vec(sz)},true);"

    parts = []
    for pl in (center[L] - deck_len/2 + pillar_tl/2, center[L] + deck_len/2 - pillar_tl/2):
        parts.append(box(pl, pillar_tl, base + pillar_h/2, pillar_h, pillar_tw))
    parts.append(box(center[L], deck_len, base + pillar_h + deck_t/2, deck_t, deck_w))

    if rotate is None:
        return "union() { " + " ".join(parts) + " }"   # primitives give the exact training voxels
    vox = _bridge_voxels(deck_len, deck_w, deck_t, pillar_tl, pillar_tw, pillar_h, up_axis, center, G)
    return _voxels_csg(_rotate_voxels(vox, rotate, G))


def main():
    experiments = [
        # --- SPHERE (20 cases) ---
        ("Sphere", "radius 3.0 (small, in-dist)", "translate([12,12,12]) sphere(3.0);"),
        ("Sphere", "radius 4.5 (medium, in-dist)", "translate([12,12,12]) sphere(4.5);"),
        ("Sphere", "radius 6.0 (large, in-dist)", "translate([12,12,12]) sphere(6.0);"),
        ("Sphere", "radius 2.0 (too small, out-dist)", "translate([12,12,12]) sphere(2.0);"),
        ("Sphere", "radius 8.0 (large, out-dist)", "translate([12,12,12]) sphere(8.0);"),
        ("Sphere", "radius 12.0 (oversized, clipping)", "translate([12,12,12]) sphere(12.0);"),
        ("Sphere", "scaled spheroid (1.2, 0.8, 1.0)", "translate([12,12,12]) scale([1.2,0.8,1.0]) sphere(5);"),
        ("Sphere", "oblate spheroid (1.1, 0.8, 0.6)", "translate([12,12,12]) scale([1.1,0.8,0.6]) sphere(7);"),
        ("Sphere", "rotated & scaled", "translate([12,12,12]) rotate([0,45,45]) scale([1.2,0.9,0.9]) sphere(5);"),
        ("Sphere", "translated off-center", "translate([9,15,11]) sphere(5);"),
        ("Sphere", "radius 2.5 (small boundary)", "translate([12,12,12]) sphere(2.5);"),
        ("Sphere", "radius 1.5 (very small)", "translate([12,12,12]) sphere(1.5);"),
        ("Sphere", "radius 10.0 (very large)", "translate([12,12,12]) sphere(10.0);"),
        ("Sphere", "radius 11.5 (huge limit-scale)", "translate([12,12,12]) sphere(11.5);"),
        ("Sphere", "highly oval spheroid (scale 2.0, 0.8, 0.8)", "translate([12,12,12]) scale([2.0,0.8,0.8]) sphere(5);"),
        ("Sphere", "deformed spheroid (scale 1.8, 1.5, 0.6)", "translate([12,12,12]) scale([1.8,1.5,0.6]) sphere(6);"),
        ("Sphere", "huge sphere (r15.0)", "translate([25,25,25]) sphere(15.0);"),
        ("Sphere", "huge sphere (r20.0)", "translate([25,25,25]) sphere(20.0);"),
        ("Sphere", "huge oval sphere (scale 1.8, 0.8, 1.0, r12)", "translate([25,25,25]) scale([1.8,0.8,1.0]) sphere(12.0);"),
        ("Sphere", "huge sphere (r13.0)", "translate([25,25,25]) sphere(13.0);"),

        # --- CUBE (20 cases) ---
        ("Cube", "size 4 (small, in-dist)", "translate([12,12,12]) cube([4,4,4], true);"),
        ("Cube", "size 6 (medium, in-dist)", "translate([12,12,12]) cube([6,6,6], true);"),
        ("Cube", "size 8 (medium, in-dist)", "translate([12,12,12]) cube([8,8,8], true);"),
        ("Cube", "size 10 (large, in-dist)", "translate([12,12,12]) cube([10,10,10], true);"),
        ("Cube", "size 3 (too small, out-dist)", "translate([12,12,12]) cube([3,3,3], true);"),
        ("Cube", "size 12 (large, out-dist)", "translate([12,12,12]) cube([12,12,12], true);"),
        ("Cube", "size 16 (oversized, clipping)", "translate([12,12,12]) cube([16,16,16], true);"),
        ("Cube", "scaled rectangular (1.3, 0.8, 1.0)", "translate([12,12,12]) scale([1.3,0.8,1.0]) cube([8,8,8], true);"),
        ("Cube", "rotated 45deg Z", "translate([12,12,12]) rotate([0,0,45]) cube([8,8,8], true);"),
        ("Cube", "rotated 30deg X & Y", "translate([12,12,12]) rotate([30,30,0]) cube([8,8,8], true);"),
        ("Cube", "rotated & scaled", "translate([12,12,12]) rotate([15,30,45]) scale([1.1,0.9,1.0]) cube([8,8,8], true);"),
        ("Cube", "translated off-center", "translate([14,10,12]) cube([8,8,8], true);"),
        ("Cube", "rotated 15deg all axes", "translate([12,12,12]) rotate([15,15,15]) cube([8,8,8], true);"),
        ("Cube", "rotated 60deg Z", "translate([12,12,12]) rotate([0,0,60]) cube([8,8,8], true);"),
        ("Cube", "rotated 90deg X", "translate([12,12,12]) rotate([90,0,0]) cube([8,8,8], true);"),
        ("Cube", "rotated 45deg X & Y", "translate([12,12,12]) rotate([45,45,0]) cube([8,8,8], true);"),
        ("Cube", "size 3.5 (small boundary)", "translate([12,12,12]) cube([3.5,3.5,3.5], true);"),
        ("Cube", "size 5 rotated 30deg all", "translate([12,12,12]) rotate([30,30,30]) cube([5,5,5], true);"),
        ("Cube", "non-uniform 10x4x6", "translate([12,12,12]) cube([10,4,6], true);"),
        ("Cube", "scaled cube (scale 2.0 all)", "translate([12,12,12]) scale([2.0,2.0,2.0]) cube([5,5,5], true);"),

        # --- CYLINDER (20 cases) ---
        ("Cylinder", "r3 h8 (small, in-dist)", "translate([12,12,8]) cylinder(8, 3, 3, true);"),
        ("Cylinder", "r4.5 h11 (medium, in-dist)", "translate([12,12,6]) cylinder(11, 4.5, 4.5, true);"),
        ("Cylinder", "r1.5 h5 (too small, out-dist)", "translate([12,12,10]) cylinder(5, 1.5, 1.5, true);"),
        ("Cylinder", "r7 h15 (large, out-dist)", "translate([12,12,5]) cylinder(15, 7, 7, true);"),
        ("Cylinder", "elliptical scale (0.8, 1.2, 1.0)", "translate([12,12,6]) scale([0.8,1.2,1.0]) cylinder(12, 4, 4, true);"),
        ("Cylinder", "rotated 45deg X", "translate([12,12,12]) rotate([45,0,0]) cylinder(12, 4, 4, true);"),
        ("Cylinder", "rotated & scaled", "translate([12,12,12]) rotate([15,30,45]) scale([0.9,1.1,1.0]) cylinder(10, 4, 4, true);"),
        ("Cylinder", "translated off-center", "translate([10,14,12]) cylinder(10, 4, 4, true);"),
        ("Cylinder", "rotated 90deg X (horizontal)", "translate([12,12,12]) rotate([90,0,0]) cylinder(10, 4, 4, true);"),
        ("Cylinder", "rotated 45deg Y", "translate([12,12,12]) rotate([0,45,0]) cylinder(10, 4, 4, true);"),
        ("Cylinder", "rotated 30deg X & Z", "translate([12,12,12]) rotate([30,0,30]) cylinder(10, 4, 4, true);"),
        ("Cylinder", "r2 h12 (thin tall)", "translate([12,12,6]) cylinder(12, 2, 2, true);"),
        ("Cylinder", "r5 h4 (flat wide)", "translate([12,12,10]) cylinder(4, 5, 5, true);"),
        ("Cylinder", "scaled cylinder (scale 2.0 all)", "translate([12,12,6]) scale([2.0,2.0,2.0]) cylinder(8, 3, 3, true);"),
        ("Cylinder", "elliptical highly oval", "translate([12,12,6]) scale([1.8,0.6,1.2]) cylinder(10, 4, 4, true);"),
        ("Cylinder", "huge cylinder (r8 h30)", "translate([25,25,10]) cylinder(30, 8, 8, true);"),
        ("Cylinder", "huge cylinder (r12 h40)", "translate([25,25,5]) cylinder(40, 12, 12, true);"),
        ("Cylinder", "huge oval cylinder", "translate([25,25,12]) scale([1.8,0.8,1.0]) cylinder(25, 10, 10, true);"),
        ("Cylinder", "huge cylinder (r9 h25)", "translate([25,25,12]) cylinder(25, 9, 9, true);"),
        ("Cylinder", "huge cylinder (r10 h35)", "translate([25,25,7]) cylinder(35, 10, 10, true);"),

        # --- CHAIR (15 cases) ---
        ("Chair", "small chair", "translate([8,8,5]) cube([4,4,0.8],true); translate([6,8,7]) cube([0.8,4,4],true); translate([6,6,3]) cube([0.8,0.8,4],true); translate([6,10,3]) cube([0.8,0.8,4],true); translate([10,6,3]) cube([0.8,0.8,4],true); translate([10,10,3]) cube([0.8,0.8,4],true);"),
        ("Chair", "medium chair (default)", "translate([8,8,6]) cube([6,6,1.2],true); translate([5,8,9]) cube([1.2,6,6],true); translate([5,5,3]) cube([1.2,1.2,6.0],true); translate([5,11,3]) cube([1.2,1.2,6.0],true); translate([11,5,3]) cube([1.2,1.2,6.0],true); translate([11,11,3]) cube([1.2,1.2,6.0],true);"),
        ("Chair", "large chair", "translate([12,12,10]) cube([10,10,2],true); translate([7,12,15]) cube([2,10,10],true); translate([7,7,5]) cube([2,2,10],true); translate([7,17,5]) cube([2,2,10],true); translate([17,7,5]) cube([2,2,10],true); translate([17,17,5]) cube([2,2,10],true);"),
        ("Chair", "rotated chair 45deg Z", "translate([12,12,6]) rotate([0,0,45]) cube([6,6,1.2],true); translate([12,12,6]) rotate([0,0,45]) translate([-2.4,0,3]) cube([1.2,6,6],true); translate([12,12,6]) rotate([0,0,45]) translate([-2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([0,0,45]) translate([-2.4,2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([0,0,45]) translate([2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([0,0,45]) translate([2.4,2.4,-3]) cube([1.2,1.2,6.0],true);"),
        ("Chair", "rotated chair 90deg Z", "translate([12,12,6]) rotate([0,0,90]) cube([6,6,1.2],true); translate([12,12,6]) rotate([0,0,90]) translate([-2.4,0,3]) cube([1.2,6,6],true); translate([12,12,6]) rotate([0,0,90]) translate([-2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([0,0,90]) translate([-2.4,2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([0,0,90]) translate([2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([0,0,90]) translate([2.4,2.4,-3]) cube([1.2,1.2,6.0],true);"),
        ("Chair", "slanted chair", "translate([12,12,6]) rotate([10,15,0]) cube([6,6,1.2],true); translate([12,12,6]) rotate([10,15,0]) translate([-2.4,0,3]) cube([1.2,6,6],true); translate([12,12,6]) rotate([10,15,0]) translate([-2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([10,15,0]) translate([-2.4,2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([10,15,0]) translate([2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([10,15,0]) translate([2.4,2.4,-3]) cube([1.2,1.2,6.0],true);"),
        ("Chair", "translated chair", "translate([9,15,8]) cube([6,6,1.2],true); translate([9,15,8]) translate([-2.4,0,3]) cube([1.2,6,6],true); translate([9,15,8]) translate([-2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([9,15,8]) translate([-2.4,2.4,-3]) cube([1.2,1.2,6.0],true); translate([9,15,8]) translate([2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([9,15,8]) translate([2.4,2.4,-3]) cube([1.2,1.2,6.0],true);"),
        ("Chair", "scaled chair (wide seat)", "translate([12,12,6]) cube([8,6,1.2],true); translate([12,12,6]) translate([-3.4,0,3]) cube([1.2,6,6],true); translate([12,12,6]) translate([-3.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) translate([-3.4,2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) translate([3.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) translate([3.4,2.4,-3]) cube([1.2,1.2,6.0],true);"),
        ("Chair", "huge chair in 50x50x50 grid", "translate([25,25,20]) cube([20,20,4],true); translate([25,25,20]) translate([-8,0,10]) cube([4,20,20],true); translate([25,25,20]) translate([-8,-8,-10]) cube([4,4,20],true); translate([25,25,20]) translate([-8,8,-10]) cube([4,4,20],true); translate([25,25,20]) translate([8,-8,-10]) cube([4,4,20],true); translate([25,25,20]) translate([8,8,-10]) cube([4,4,20],true);"),
        ("Chair", "huge rotated chair Z", "translate([25,25,20]) rotate([0,0,30]) cube([20,20,4],true); translate([25,25,20]) rotate([0,0,30]) translate([-8,0,10]) cube([4,20,20],true); translate([25,25,20]) rotate([0,0,30]) translate([-8,-8,-10]) cube([4,4,20],true); translate([25,25,20]) rotate([0,0,30]) translate([-8,8,-10]) cube([4,4,20],true); translate([25,25,20]) rotate([0,0,30]) translate([8,-8,-10]) cube([4,4,20],true); translate([25,25,20]) rotate([0,0,30]) translate([8,8,-10]) cube([4,4,20],true);"),
        ("Chair", "slanted medium chair", "translate([12,12,6]) rotate([5,-5,5]) cube([6,6,1.2],true); translate([12,12,6]) rotate([5,-5,5]) translate([-2.4,0,3]) cube([1.2,6,6],true); translate([12,12,6]) rotate([5,-5,5]) translate([-2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([5,-5,5]) translate([-2.4,2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([5,-5,5]) translate([2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) rotate([5,-5,5]) translate([2.4,2.4,-3]) cube([1.2,1.2,6.0],true);"),
        ("Chair", "tall back chair", "translate([12,12,6]) cube([6,6,1.2],true); translate([12,12,6]) translate([-2.4,0,5]) cube([1.2,6,10],true); translate([12,12,6]) translate([-2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) translate([-2.4,2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) translate([2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) translate([2.4,2.4,-3]) cube([1.2,1.2,6.0],true);"),
        ("Chair", "thin legs chair", "translate([12,12,6]) cube([6,6,1.2],true); translate([12,12,6]) translate([-2.4,0,3]) cube([1.2,6,6],true); translate([12,12,6]) translate([-2.4,-2.4,-3]) cube([0.6,0.6,6.0],true); translate([12,12,6]) translate([-2.4,2.4,-3]) cube([0.6,0.6,6.0],true); translate([12,12,6]) translate([2.4,-2.4,-3]) cube([0.6,0.6,6.0],true); translate([12,12,6]) translate([2.4,2.4,-3]) cube([0.6,0.6,6.0],true);"),
        ("Chair", "armrest chair", "translate([12,12,6]) cube([6,6,1.2],true); translate([12,12,6]) translate([-2.4,0,3]) cube([1.2,6,6],true); translate([12,12,6]) translate([-2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) translate([-2.4,2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) translate([2.4,-2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) translate([2.4,2.4,-3]) cube([1.2,1.2,6.0],true); translate([12,12,6]) translate([0,-2.4,3]) cube([6,0.8,2],true); translate([12,12,6]) translate([0,2.4,3]) cube([6,0.8,2],true);"),
        ("Chair", "bar stool chair (tall legs)", "translate([12,12,10]) cube([5,5,1.2],true); translate([12,12,10]) translate([-2.0,0,2]) cube([1.0,5,4],true); translate([12,12,10]) translate([-2.0,-2.0,-5]) cube([1.0,1.0,10.0],true); translate([12,12,10]) translate([-2.0,2.0,-5]) cube([1.0,1.0,10.0],true); translate([12,12,10]) translate([2.0,-2.0,-5]) cube([1.0,1.0,10.0],true); translate([12,12,10]) translate([2.0,2.0,-5]) cube([1.0,1.0,10.0],true);"),

        # --- BRIDGE (15 cases, training-matched, 16^3 world) ---
        # (shape, description, csg, grid). Parameters lie inside the training ranges and every
        # shape is a single face-connected piece: the simulator only aggregates the component that
        # contains block 1, while training averages over all voxels, so fragmented shapes are not
        # comparable. Rotation angles were chosen so the training-style rotation stays connected.
        ("Bridge", "medium bridge",               bridge_train_csg(8.3, 3.2, 0.9, 0.9, 3.0, 4.2, 2, (7.6, 8.3, 7.75)), TRAIN_GRID),
        ("Bridge", "small bridge",                bridge_train_csg(4.6, 1.8, 0.7, 0.7, 1.7, 2.4, 2, (8.2, 7.7, 8.1)), TRAIN_GRID),
        ("Bridge", "long tall bridge",            bridge_train_csg(10.6, 4.4, 1.1, 1.1, 4.2, 5.6, 2, (8.1, 7.9, 8.2)), TRAIN_GRID),
        ("Bridge", "bridge up-axis X",            bridge_train_csg(7.4, 2.7, 0.8, 1.0, 2.6, 3.6, 0, (7.55, 8.15, 7.75)), TRAIN_GRID),
        ("Bridge", "bridge up-axis Y",            bridge_train_csg(9.2, 3.8, 1.0, 1.0, 3.5, 4.8, 1, (8.3, 7.7, 8.0)), TRAIN_GRID),
        ("Bridge", "rotated (150,260,330)",       bridge_train_csg(8.3, 3.2, 0.9, 0.9, 3.0, 4.2, 2, (7.6, 8.3, 7.75), rotate=(150, 260, 330)), TRAIN_GRID),
        ("Bridge", "rotated (250,280,5)",         bridge_train_csg(8.3, 3.2, 0.9, 0.9, 3.0, 4.2, 2, (7.6, 8.3, 7.75), rotate=(250, 280, 5)), TRAIN_GRID),
        ("Bridge", "rotated (75,120,225)",        bridge_train_csg(9.2, 3.8, 1.0, 1.0, 3.5, 4.8, 2, (8.3, 7.7, 8.0), rotate=(75, 120, 225)), TRAIN_GRID),
        ("Bridge", "rotated (245,50,355)",        bridge_train_csg(10.6, 4.4, 1.1, 1.1, 4.2, 5.6, 2, (8.1, 7.9, 8.2), rotate=(245, 50, 355)), TRAIN_GRID),
        ("Bridge", "rotated (275,140,175)",       bridge_train_csg(10.6, 4.4, 1.1, 1.1, 4.2, 5.6, 2, (8.1, 7.9, 8.2), rotate=(275, 140, 175)), TRAIN_GRID),
        ("Bridge", "overflowing x1.8",            bridge_train_csg(12.6, 6.3, 1.62, 1.62, 3.6, 9.0, 2, (8.0, 8.0, 7.0)), TRAIN_GRID),
        ("Bridge", "overflowing x2.5",            bridge_train_csg(20.0, 7.5, 2.5, 2.5, 5.0, 15.0, 2, (8.5, 7.5, 9.0)), TRAIN_GRID),
        ("Bridge", "overflowing x3.2 up-axis X",  bridge_train_csg(16.0, 6.4, 1.6, 1.6, 3.2, 9.6, 0, (7.0, 8.0, 8.0)), TRAIN_GRID),
        ("Bridge", "overflowing x1.6 up-axis Y",  bridge_train_csg(9.6, 4.0, 1.12, 1.28, 2.4, 6.4, 1, (7.0, 8.0, 9.0)), TRAIN_GRID),
        ("Bridge", "overflowing x2.2 off-centre", bridge_train_csg(19.8, 8.8, 1.76, 2.2, 6.6, 12.1, 2, (3.5, 12.0, 8.0)), TRAIN_GRID),

        # --- HUMANOID (10 cases, training-matched stick figure, 16^3 world) ---
        ("Humanoid", "standing, arms down",       humanoid_train_csg((4.7, 3.0, 1.8), 1.55, (4.2, 0.85, (0, 0), (0, 0)), (4.2, 0.75, (10, 0), (-10, 0))), TRAIN_GRID),
        ("Humanoid", "arms raised",               humanoid_train_csg((4.7, 3.0, 1.8), 1.55, (4.2, 0.85, (0, 0), (0, 0)), (4.2, 0.75, (150, 0), (-150, 0))), TRAIN_GRID),
        ("Humanoid", "T-pose",                    humanoid_train_csg((4.7, 3.0, 1.8), 1.55, (4.2, 0.85, (0, 0), (0, 0)), (4.2, 0.75, (90, 0), (-90, 0))), TRAIN_GRID),
        ("Humanoid", "walking stride",            humanoid_train_csg((4.7, 3.0, 1.8), 1.55, (4.2, 0.85, (5, 30), (-5, -20)), (4.2, 0.75, (0, -30), (0, 40))), TRAIN_GRID),
        ("Humanoid", "small, thin limbs",         humanoid_train_csg((4.1, 2.6, 1.6), 1.35, (3.6, 0.72, (0, 5), (0, -5)), (3.6, 0.62, (20, 10), (-20, 10))), TRAIN_GRID),
        ("Humanoid", "large, thick limbs",        humanoid_train_csg((5.4, 3.4, 2.1), 1.75, (4.9, 0.98, (-8, 0), (8, 0)), (4.9, 0.88, (35, 20), (-35, 20))), TRAIN_GRID),
        ("Humanoid", "rotated (135,95,350)",      humanoid_train_csg((4.7, 3.0, 1.8), 1.55, (4.2, 0.85, (0, 0), (0, 0)), (4.2, 0.75, (10, 0), (-10, 0)), rotate=(135, 95, 350)), TRAIN_GRID),
        ("Humanoid", "lying down, arms raised",   humanoid_train_csg((4.7, 3.0, 1.8), 1.55, (4.2, 0.85, (0, 0), (0, 0)), (4.2, 0.75, (150, 0), (-150, 0)), rotate=(95, 270, 95)), TRAIN_GRID),
        ("Humanoid", "overflowing zoom 1.5",      humanoid_train_csg((4.7, 3.0, 1.8), 1.55, (4.2, 0.85, (0, 0), (0, 0)), (4.2, 0.75, (10, 0), (-10, 0)), zoom=1.5), TRAIN_GRID),
        ("Humanoid", "overflowing zoom 2.2 T-pose", humanoid_train_csg((4.7, 3.0, 1.8), 1.55, (4.2, 0.85, (0, 0), (0, 0)), (4.2, 0.75, (90, 0), (-90, 0)), zoom=2.2), TRAIN_GRID),
    ]

    fieldnames = [
        "Actual Shape", 
        "Description", 
        "Grid",
        "Predicted", 
        "Confidence", 
        "Blocks", 
        "Robots",
        "Total Messages", 
        "Total Bandwidth (bytes)",
        "Simulation Time (us)", 
        "Cube %", 
        "Sphere %", 
        "Cylinder %", 
        "Chair %",
        "Bridge %",
        "Humanoid %",
        "CSG Code"
    ]

    # Always overwrite CSV output with header on start of run
    with open(CSV_OUTPUT, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()

    correct_counts = {"Cube": 0, "Sphere": 0, "Cylinder": 0, "Chair": 0, "Bridge": 0, "Humanoid": 0}
    total_counts = {"Cube": 0, "Sphere": 0, "Cylinder": 0, "Chair": 0, "Bridge": 0, "Humanoid": 0}

    if not os.path.isfile(EXE_PATH):
        sys.exit(f"Simulator not found at {EXE_PATH}. Build it with: cmake --build build --target AI4PM_DeepEmb3D")

    tmp_fd, tmp_config = tempfile.mkstemp(prefix='deep_v5_', suffix='.xml')
    os.close(tmp_fd)

    for i, (shape, desc, csg, *grid) in enumerate(experiments, 1):
        grid = grid[0] if grid else 50
        print(f"\n--- EXPERIMENT {i}/{len(experiments)}: {shape} ({desc}) ---")
        write_config(csg, tmp_config, grid)
        raw_output = run_simulation(tmp_config)
        data = parse_output(raw_output, shape, desc, csg)
        data["Grid"] = grid
        
        with open(CSV_OUTPUT, 'a', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=fieldnames)
            writer.writerow(data)
        
        print(f"Result: Predicted {data['Predicted']} with {data['Confidence']}% confidence.")
        if data["Robots"] and data["Blocks"] < data["Robots"]:
            print(f"[WARNING] Only {data['Blocks']}/{data['Robots']} blocks aggregated: the shape is not connected")
        
        total_counts[shape] += 1
        if data["Predicted"].lower() == shape.lower():
            correct_counts[shape] += 1
            
    print("\n" + "="*50)
    print("EXPERIMENT SUMMARY")
    print("="*50)
    for shape in total_counts.keys():
        if total_counts[shape] > 0:
            acc = (correct_counts[shape] / total_counts[shape]) * 100
            print(f"{shape:10s}: {correct_counts[shape]}/{total_counts[shape]} correct ({acc:.1f}%)")
    total_ok, total_n = sum(correct_counts.values()), sum(total_counts.values())
    print(f"{'Overall':10s}: {total_ok}/{total_n} correct ({100 * total_ok / max(total_n, 1):.1f}%)")
    print(f"Logged to {CSV_OUTPUT}")
    os.remove(tmp_config)

if __name__ == "__main__":
    main()
