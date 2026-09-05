// Pin mapping cho camera OV2640 trên board ESP32-S3-CAM (Freenove ESP32-S3-WROOM CAM).
//
// ⚠️ QUAN TRỌNG: Đây là pin mapping tham khảo theo mẫu "CAMERA_MODEL_ESP32S3_EYE" phổ biến
// trong các ví dụ esp32-camera chính thức. Board Freenove có thể dùng revision pin KHÁC.
// TRƯỚC KHI FLASH THẬT: đối chiếu và thay bằng pin mapping chính xác lấy từ ví dụ đi kèm
// board bạn mua (thường nằm trong SDK/tài liệu sản phẩm của Freenove, hoặc sơ đồ chân trên
// trang bán hàng). Đây là phần bắt buộc phải xác nhận với phần cứng thật, chưa thể kiểm chứng
// khi chưa có board (xem Phase 1 trong kế hoạch redesign).

#pragma once

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     15
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5

#define Y9_GPIO_NUM       16
#define Y8_GPIO_NUM       17
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       12
#define Y5_GPIO_NUM       10
#define Y4_GPIO_NUM       8
#define Y3_GPIO_NUM       9
#define Y2_GPIO_NUM       11
#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM     13

#define LED_GPIO_NUM      -1
