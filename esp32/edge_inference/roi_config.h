// Cấu hình khung ảnh & vùng ROI dùng chung cho capture_mode.ino (thu thập dữ liệu)
// và edge_inference.ino (suy luận chính thức) — phải khớp với giá trị hardcode
// tương ứng trong pc/collect_calibration_set.py khi thu thập dữ liệu hiệu chỉnh.

#pragma once

#define CAM_FRAME_WIDTH   160   // FRAMESIZE_QQVGA
#define CAM_FRAME_HEIGHT  120
#define ROI_SIZE          120   // Cắt vùng vuông ở giữa khung hình (đặt tay vào giữa)
#define MODEL_INPUT_SIZE  48    // Phải khớp IMG_SIZE trong pc/train_model.py
