"""
quantize_deep_embedded3d.py
============================
Weight-only INT8 quantization for the DeepEmb3D distributed 3D-CNN header.

WHAT THIS DOES
---------------
Reads an existing float32 C header exported by
`tinyml_improved_deep_embedded3d_v5.py` (e.g. the "(13)" header that gave the
best Colab/VisibleSim results), quantizes the 5 weight tensors to int8 with
one scale per OUTPUT channel (standard per-channel symmetric quantization),
leaves the small bias vectors as float32, and writes a new header with the
same layer names (`_q` / `_scale` suffixes) that is a drop-in replacement
for the current float32 header, modulo a small firmware change (see the
bottom of this file's printed summary).

Only weights shrink; activations and all arithmetic stay float32 exactly as
today, so the block-to-block message format (feature vectors, K floats) does
NOT change and the distributed message-passing logic in
user_code_AI4PM3D.c does NOT need to change — only the 5 weight arrays and
the dot-product loops that read them.

WHY NOT A FULL TFLITE INT8 CONVERSION (like the 2D/ANN pipeline)?
-------------------------------------------------------------------
The 2D pipeline (tinyml_ai4pm.py / tinyml_ai4pm_cnn.py) runs the whole Keras
graph through TFLiteConverter with TFLITE_BUILTINS_INT8, quantizing both
weights AND activations, then extracts per-layer tensors from the resulting
.tflite flatbuffer. That requires every op in the graph (including the
custom FaceGatherLayer's pad/slice/concat and the residual adds) to convert
cleanly to full-integer TFLite kernels, and would require rewriting the
firmware's message-passing code to carry int8 activations with zero-points
and requantization. Weight-only quantization gets the same flash-size win
(4x) without touching the graph or the on-wire message format, so it's the
lower-risk choice for this already-complex distributed firmware.

HOW TO RUN
-----------
1. Upload `neuralNetworkData_deep_embedded3d (13).h` (or your chosen header)
   to the Colab session.
2. Set HEADER_PATH below to that filename.
3. (Optional but recommended) Also upload/paste
   `tinyml_improved_deep_embedded3d_v5.py` into the same Colab session and
   run it up through the function definitions (build_model, data_generator,
   CLASS_NAMES, GRID, K, K_FC1, NUM_CLASSES) WITHOUT calling
   train_and_export() again. This lets the script rebuild the exact Keras
   model, load the parsed float32 weights into it, and measure the real
   accuracy delta between the float32 and quantized weights on freshly
   generated validation shapes before you commit to a hardware test.
   If you skip this, the script still parses, quantizes, and exports the
   header — it just can't report a real accuracy number, only the raw
   per-tensor quantization error.
4. Run this script (paste as a new cell, or `!python quantize_deep_embedded3d.py`).
5. Download the exported header and the printed firmware-change checklist.
"""

import re
import numpy as np

HEADER_PATH = "neuralNetworkData_deep_embedded3d (13).h"
OUTPUT_HEADER = "neuralNetworkData_deep_embedded3d_int8.h"

# Names + declared Keras-layer name (for the optional accuracy check) of
# every weight-bearing tensor. Biases are listed separately and are NOT
# quantized (they're tiny: ~150 floats total across the whole model).
WEIGHT_TENSORS = [
    "conv1_3d_kernel",
    "conv2_face_weight",
    "conv3_face_weight",
    "fc1_deep_weight",
    "fc2_deep_weight",
]
BIAS_TENSORS = [
    "conv1_3d_bias",
    "conv2_face_bias",
    "conv3_face_bias",
    "fc1_deep_bias",
    "fc2_deep_bias",
]
ALL_TENSORS = WEIGHT_TENSORS + BIAS_TENSORS

LAYER_NAME_OF = {
    "conv1_3d_kernel": "conv1_3d", "conv1_3d_bias": "conv1_3d",
    "conv2_face_weight": "conv2_face", "conv2_face_bias": "conv2_face",
    "conv3_face_weight": "conv3_face", "conv3_face_bias": "conv3_face",
    "fc1_deep_weight": "fc1_deep", "fc1_deep_bias": "fc1_deep",
    "fc2_deep_weight": "fc2_deep", "fc2_deep_bias": "fc2_deep",
}

QMAX = 127  # symmetric int8 range: [-127, 127] (skip -128 so it's symmetric)


# =============================================================================
# STEP 1 — Parse the existing float32 C header
# =============================================================================

def parse_header_arrays(header_text, names):
    """Extract each named `static const float NAME[dims...] = { ... };` array
    as a numpy float32 array with the declared shape. Relies on every float
    literal being written as `-?\\d+\\.\\d+f` (no scientific notation), which
    is exactly what export_c_header()'s `.8f` formatting produces."""
    arrays = {}
    for name in names:
        decl_pattern = r'static const float ' + re.escape(name) + r'((?:\[\d+\])+)\s*=\s*\{'
        m = re.search(decl_pattern, header_text)
        if not m:
            raise ValueError(f"Array '{name}' not found in header at {HEADER_PATH!r}")
        dims = [int(d) for d in re.findall(r'\[(\d+)\]', m.group(1))]

        # Walk forward from the opening brace to find its matching close,
        # tracking nesting depth (conv1_3d_kernel has nested braces).
        start = m.end() - 1
        depth = 0
        end = None
        for i in range(start, len(header_text)):
            c = header_text[i]
            if c == '{':
                depth += 1
            elif c == '}':
                depth -= 1
                if depth == 0:
                    end = i
                    break
        if end is None:
            raise ValueError(f"Unbalanced braces parsing '{name}'")

        block = header_text[start:end + 1]
        floats = re.findall(r'-?\d+\.\d+f', block)
        vals = np.array([float(v[:-1]) for v in floats], dtype=np.float32)

        expected = int(np.prod(dims))
        if vals.size != expected:
            raise ValueError(
                f"'{name}': expected {expected} floats from shape {dims}, "
                f"parsed {vals.size} — header format may not match what "
                f"this script expects."
            )
        arrays[name] = vals.reshape(dims)
    return arrays


# =============================================================================
# STEP 2 — Per-output-channel symmetric INT8 quantization
# =============================================================================

def quantize_per_output_channel(w, out_axis=-1):
    """Quantize `w` independently per slice along `out_axis` (the
    output-channel axis). Returns (int8 array, float32 scale array of
    length w.shape[out_axis], dequantized float32 array for accuracy
    checks). `out_axis` defaults to the last axis (correct for the Dense
    weight matrices, shape [in][out]) but must be passed explicitly for
    conv1_3d_kernel, whose output/filter axis is axis 0 — shape
    [K][1][3][3][3], where the trailing [3][3][3] are kernel *spatial*
    dims, not channels."""
    w = np.asarray(w, dtype=np.float32)
    out_axis = out_axis % w.ndim
    reduce_axes = tuple(i for i in range(w.ndim) if i != out_axis)
    max_abs = np.max(np.abs(w), axis=reduce_axes, keepdims=True)
    max_abs = np.maximum(max_abs, 1e-8)  # guard a dead output channel
    scale_keepdims = max_abs / QMAX

    q = np.clip(np.round(w / scale_keepdims), -QMAX, QMAX).astype(np.int8)
    dequant = q.astype(np.float32) * scale_keepdims
    scale_flat = scale_keepdims.reshape(-1).astype(np.float32)
    return q, scale_flat, dequant


# Output/filter axis per tensor. Only conv1_3d_kernel differs (its output
# axis is 0, not -1) — see quantize_per_output_channel's docstring.
OUT_AXIS = {
    "conv1_3d_kernel": 0,
    "conv2_face_weight": -1,
    "conv3_face_weight": -1,
    "fc1_deep_weight": -1,
    "fc2_deep_weight": -1,
}


def quantize_all(arrays):
    q_arrays, scales, dequant_arrays = {}, {}, {}
    for name in WEIGHT_TENSORS:
        q, s, dq = quantize_per_output_channel(arrays[name], out_axis=OUT_AXIS[name])
        q_arrays[name] = q
        scales[name] = s
        dequant_arrays[name] = dq
    return q_arrays, scales, dequant_arrays


# =============================================================================
# STEP 3 (optional) — Real accuracy check via the training script's model
# =============================================================================

def try_accuracy_check(arrays, dequant_arrays):
    try:
        import tensorflow as tf  # noqa: F401
        from tensorflow import keras
        build_model = globals().get("build_model")
        data_generator = globals().get("data_generator")
        CLASS_NAMES = globals().get("CLASS_NAMES")
        if build_model is None or data_generator is None:
            raise NameError
    except (ImportError, NameError):
        print(
            "\n" + "!" * 65 +
            "\n[accuracy check SKIPPED] build_model/data_generator not found "
            "in this session.\nA size check alone does NOT prove the "
            "quantized weights still work — run\ntinyml_improved_deep_embedded3d_v5.py's "
            "function-definition cells first (no need\nto retrain) so this "
            "script can measure a real accuracy delta before you\nspend a "
            "hardware test on it.\n" + "!" * 65
        )
        return

    def set_weights(model, w):
        c1_w = w["conv1_3d_kernel"].transpose(2, 3, 4, 1, 0)  # -> Keras (3,3,3,1,K)
        model.get_layer("conv1_3d").set_weights([c1_w, arrays["conv1_3d_bias"]])
        model.get_layer("conv2_face").set_weights([w["conv2_face_weight"], arrays["conv2_face_bias"]])
        model.get_layer("conv3_face").set_weights([w["conv3_face_weight"], arrays["conv3_face_bias"]])
        model.get_layer("fc1_deep").set_weights([w["fc1_deep_weight"], arrays["fc1_deep_bias"]])
        model.get_layer("fc2_deep").set_weights([w["fc2_deep_weight"], arrays["fc2_deep_bias"]])

    model = build_model()

    print("\n[accuracy check] Generating a fresh validation set...")
    X_val, y_val = data_generator(3000, seed=12345, is_validation=True)

    fp32_w = {name: arrays[name] for name in WEIGHT_TENSORS}
    set_weights(model, fp32_w)
    fp32_loss, fp32_acc = model.evaluate(X_val, y_val, verbose=0)
    fp32_preds = model.predict(X_val, verbose=0)
    print(f"  FP32 (parsed header) accuracy:      {fp32_acc*100:.2f}%  "
          f"(sanity check that header parsing round-trips correctly —\n"
          f"                                       should match what you saw in "
          f"training/VisibleSim for this header)")

    set_weights(model, dequant_arrays)
    q_loss, q_acc = model.evaluate(X_val, y_val, verbose=0)
    q_preds = model.predict(X_val, verbose=0)
    print(f"  INT8 weight-only quantized accuracy: {q_acc*100:.2f}%")
    print(f"  Accuracy delta:                      {(q_acc-fp32_acc)*100:+.2f} pts")

    true_c = y_val.argmax(axis=1)
    fp32_pred_c = fp32_preds.argmax(axis=1)
    q_pred_c = q_preds.argmax(axis=1)
    names = CLASS_NAMES or [str(i) for i in range(fp32_preds.shape[1])]

    print(f"\n  {'Class':10s} {'FP32 acc':>10s} {'INT8 acc':>10s} {'delta':>8s}  n")
    for ci, cname in enumerate(names):
        mask = true_c == ci
        if mask.sum() == 0:
            continue
        fa = (fp32_pred_c[mask] == ci).mean() * 100
        qa = (q_pred_c[mask] == ci).mean() * 100
        print(f"    {cname:9s} {fa:9.1f}% {qa:9.1f}% {qa-fa:+7.1f}%  {mask.sum()}")

    # Confusion matrix for the quantized model, so a class that fully
    # collapses into another (like Conv1's zero-scale bug did into Sphere)
    # is immediately visible rather than hidden behind an overall accuracy %.
    n = len(names)
    cm = np.zeros((n, n), dtype=int)
    for t, p in zip(true_c, q_pred_c):
        cm[t, p] += 1
    print("\n  INT8 confusion matrix (rows=true, cols=predicted):")
    header = "".join(f"{c[:4]:>6s}" for c in names)
    print(f"    {'':9s} {header}")
    for ci, cname in enumerate(names):
        row = "".join(f"{cm[ci,cj]:6d}" for cj in range(n))
        print(f"    {cname:9s} {row}")

    if q_acc < fp32_acc - 0.05:
        print(
            f"\n  [WARNING] INT8 accuracy dropped more than 5 points vs FP32 "
            f"({(q_acc-fp32_acc)*100:+.1f} pts). Do not flash hardware with "
            f"this header until this is understood — check the confusion "
            f"matrix above for a class collapsing into another, which usually "
            f"means a scale/axis bug rather than ordinary quantization noise."
        )


# =============================================================================
# STEP 4 — Export the quantized C header
# =============================================================================

def format_int_array(vals):
    return ','.join(str(int(v)) for v in vals)


def format_float_array(vals):
    return ','.join(f'{v:.8f}f' for v in vals)


def export_quantized_header(arrays, q_arrays, scales, output_path):
    k = arrays["conv1_3d_bias"].shape[0]
    kfc1 = arrays["fc1_deep_bias"].shape[0]
    nc = arrays["fc2_deep_bias"].shape[0]

    with open(output_path, 'w') as f:
        f.write("// Auto-generated by quantize_deep_embedded3d.py\n")
        f.write("// Weight-only INT8 quantization of neuralNetworkData_deep_embedded3d.h\n")
        f.write("// Weights: int8_t, one scale per OUTPUT channel. Biases: unchanged float32.\n")
        f.write("// Activation/dataflow math in firmware is UNCHANGED (still float32) --\n")
        f.write("// only the weight dot-product loops need updating to dequantize on read:\n")
        f.write("//   sum += (float)W_Q[...][k] * activation[...];   // accumulate as before\n")
        f.write("//   sum *= W_SCALE[k];                              // once, after the loop\n")
        f.write("//   sum += W_BIAS[k];\n")
        f.write("#ifndef NEURALNETWORKDATA_DEEP_EMBEDDED3D_INT8_H\n")
        f.write("#define NEURALNETWORKDATA_DEEP_EMBEDDED3D_INT8_H\n\n")
        f.write("#include <stdint.h>\n\n")

        f.write(f"static const int DEEP_K          = {k};\n")
        f.write(f"static const int DEEP_K_FC1       = {kfc1};\n")
        f.write(f"static const int DEEP_NUM_CLASSES = {nc};\n")
        f.write(f"static const int DEEP_COMBINED    = {7*k};\n\n")

        # --- conv1_3d_kernel: int8 [K][1][3][3][3] + scale[K] ---
        q = q_arrays["conv1_3d_kernel"]
        f.write(f"// Phase 1: Conv3D kernel, per-filter scale. Shape: [K][1][3][3][3]\n")
        f.write(f"static const int8_t conv1_3d_kernel_q[{k}][1][3][3][3] = {{\n")
        for filt in range(k):
            f.write("  {{\n")
            for i in range(3):
                f.write("    {\n")
                for j in range(3):
                    vals = ','.join(str(int(q[filt][0][i][j][kk])) for kk in range(3))
                    f.write(f"      {{{vals}}},\n")
                f.write("    },\n")
            f.write("  }},\n")
        f.write("};\n")
        f.write(f"static const float conv1_3d_kernel_scale[{k}] = "
                 f"{{{format_float_array(scales['conv1_3d_kernel'])}}};\n\n")
        f.write(f"static const float conv1_3d_bias[{k}] = "
                 f"{{{format_float_array(arrays['conv1_3d_bias'])}}};\n\n")

        # --- conv2_face_weight: int8 [7K][K] + scale[K] ---
        q = q_arrays["conv2_face_weight"]
        f.write(f"// Phase 2: FaceGather Dense weights, per-output scale. Shape: [7K][K]\n")
        f.write(f"static const int8_t conv2_face_weight_q[{7*k}][{k}] = {{\n")
        for i in range(7 * k):
            f.write(f"  {{{format_int_array(q[i])}}},\n")
        f.write("};\n")
        f.write(f"static const float conv2_face_weight_scale[{k}] = "
                 f"{{{format_float_array(scales['conv2_face_weight'])}}};\n\n")
        f.write(f"static const float conv2_face_bias[{k}] = "
                 f"{{{format_float_array(arrays['conv2_face_bias'])}}};\n\n")

        # --- conv3_face_weight: int8 [7K][K] + scale[K] ---
        q = q_arrays["conv3_face_weight"]
        f.write(f"// Phase 3: FaceGather Dense weights, per-output scale. Shape: [7K][K]\n")
        f.write(f"static const int8_t conv3_face_weight_q[{7*k}][{k}] = {{\n")
        for i in range(7 * k):
            f.write(f"  {{{format_int_array(q[i])}}},\n")
        f.write("};\n")
        f.write(f"static const float conv3_face_weight_scale[{k}] = "
                 f"{{{format_float_array(scales['conv3_face_weight'])}}};\n\n")
        f.write(f"static const float conv3_face_bias[{k}] = "
                 f"{{{format_float_array(arrays['conv3_face_bias'])}}};\n\n")

        # --- fc1_deep_weight: int8 [K][K_FC1] + scale[K_FC1] ---
        q = q_arrays["fc1_deep_weight"]
        f.write(f"// Phase 5a: FC hidden layer, per-output scale. Shape: [K][K_FC1]\n")
        f.write(f"static const int8_t fc1_deep_weight_q[{k}][{kfc1}] = {{\n")
        for i in range(k):
            f.write(f"  {{{format_int_array(q[i])}}},\n")
        f.write("};\n")
        f.write(f"static const float fc1_deep_weight_scale[{kfc1}] = "
                 f"{{{format_float_array(scales['fc1_deep_weight'])}}};\n\n")
        f.write(f"static const float fc1_deep_bias[{kfc1}] = "
                 f"{{{format_float_array(arrays['fc1_deep_bias'])}}};\n\n")

        # --- fc2_deep_weight: int8 [K_FC1][NUM_CLASSES] + scale[NUM_CLASSES] ---
        q = q_arrays["fc2_deep_weight"]
        f.write(f"// Phase 5b: FC output layer, per-output scale. Shape: [K_FC1][{nc}]\n")
        f.write(f"static const int8_t fc2_deep_weight_q[{kfc1}][{nc}] = {{\n")
        for i in range(kfc1):
            f.write(f"  {{{format_int_array(q[i])}}},\n")
        f.write("};\n")
        f.write(f"static const float fc2_deep_weight_scale[{nc}] = "
                 f"{{{format_float_array(scales['fc2_deep_weight'])}}};\n\n")
        f.write(f"static const float fc2_deep_bias[{nc}] = "
                 f"{{{format_float_array(arrays['fc2_deep_bias'])}}};\n\n")

        f.write("#endif // NEURALNETWORKDATA_DEEP_EMBEDDED3D_INT8_H\n")


# =============================================================================
# MAIN
# =============================================================================

def main():
    print(f"Reading {HEADER_PATH} ...")
    with open(HEADER_PATH, 'r') as f:
        header_text = f.read()

    arrays = parse_header_arrays(header_text, ALL_TENSORS)

    orig_weight_bytes = sum(arrays[n].size * 4 for n in WEIGHT_TENSORS)
    orig_bias_bytes = sum(arrays[n].size * 4 for n in BIAS_TENSORS)
    orig_total = orig_weight_bytes + orig_bias_bytes
    print(f"\nParsed {len(ALL_TENSORS)} arrays.")
    print(f"  Original (float32): weights {orig_weight_bytes:,} B + "
          f"biases {orig_bias_bytes:,} B = {orig_total:,} B "
          f"({orig_total/1024:.1f} KB)")

    q_arrays, scales, dequant_arrays = quantize_all(arrays)

    print("\nPer-tensor quantization error (max / mean absolute error):")
    for name in WEIGHT_TENSORS:
        err = np.abs(arrays[name] - dequant_arrays[name])
        rel = np.abs(arrays[name]).mean() + 1e-8
        print(f"  {name:20s} max={err.max():.5f}  mean={err.mean():.5f}  "
              f"(mean |w|={rel:.5f})")

    q_weight_bytes = sum(q_arrays[n].size * 1 for n in WEIGHT_TENSORS)
    scale_bytes = sum(scales[n].size * 4 for n in WEIGHT_TENSORS)
    new_total = q_weight_bytes + scale_bytes + orig_bias_bytes
    print(f"\nQuantized: weights (int8) {q_weight_bytes:,} B + "
          f"scales (float32) {scale_bytes:,} B + biases (float32, unchanged) "
          f"{orig_bias_bytes:,} B = {new_total:,} B ({new_total/1024:.1f} KB)")
    print(f"Reduction: {orig_total/new_total:.2f}x   "
          f"{'PASSES' if new_total < 64*1024 else 'FAILS'} the 64KB target "
          f"(margin: {(64*1024-new_total)/1024:.1f} KB)")

    try_accuracy_check(arrays, dequant_arrays)

    export_quantized_header(arrays, q_arrays, scales, OUTPUT_HEADER)
    print(f"\nExported: {OUTPUT_HEADER}")
    print(
        "\nFirmware changes still needed in user_code_AI4PM3D.c before this "
        "header can be used (not applied by this script):\n"
        "  1. #include the new header instead of the float32 one.\n"
        "  2. Change conv1_3d_kernel/conv2_face_weight/conv3_face_weight/\n"
        "     fc1_deep_weight/fc2_deep_weight from `float ...` to reference\n"
        "     the `..._q` int8_t arrays plus their `..._scale` float arrays.\n"
        "  3. In each dot-product loop (compute_conv1/2/3, classify_and_broadcast),\n"
        "     accumulate `sum += (float)w_q[...] * activation` as before, then\n"
        "     AFTER the reduction loop do `sum = sum * scale[k] + bias[k]` instead\n"
        "     of `sum = w[...]*activation + ... + bias[k]` inline — the scale is\n"
        "     per output channel `k`, constant across the reduction, so it only\n"
        "     needs to be applied once per output, not per multiply-add.\n"
        "  4. Bias arrays, message formats, and all other firmware logic are\n"
        "     unchanged.\n"
    )


if __name__ == "__main__":
    main()
