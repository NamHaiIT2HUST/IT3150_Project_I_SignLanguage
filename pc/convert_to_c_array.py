"""Chuyển model/model.tflite (đã quantize int8) thành mảng C nhúng vào firmware.

Tương đương lệnh `xxd -i model.tflite` nhưng chạy được trên Windows,
không cần cài thêm công cụ ngoài.

Chạy sau khi đã có model/model.tflite (từ train_model.py):
    python pc/convert_to_c_array.py
"""
import os

MODEL_PATH = "model/model.tflite"
OUT_PATH = "../esp32/edge_inference/model_data.h"
VAR_NAME = "g_model_data"
BYTES_PER_LINE = 12

with open(MODEL_PATH, "rb") as f:
    data = f.read()

os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)

lines = []
lines.append("// File được sinh tự động bởi pc/convert_to_c_array.py — KHÔNG sửa tay.")
lines.append(f"// Nguồn: {MODEL_PATH} ({len(data)} bytes)")
lines.append("#pragma once")
lines.append("#include <cstdint>")
lines.append("")
lines.append(f"alignas(8) const unsigned char {VAR_NAME}[] = {{")

for i in range(0, len(data), BYTES_PER_LINE):
    chunk = data[i:i + BYTES_PER_LINE]
    row = ", ".join(f"0x{b:02x}" for b in chunk)
    lines.append(f"  {row},")

lines.append("};")
lines.append(f"const int {VAR_NAME}_len = {len(data)};")
lines.append("")

with open(OUT_PATH, "w") as f:
    f.write("\n".join(lines))

print(f"✅ Đã sinh {OUT_PATH} ({len(data)} bytes -> {len(data)/1024:.1f} KB)")
