// Sketch TẠM THỜI dùng để thu thập dữ liệu hiệu chỉnh (calibration set) trực tiếp từ
// camera OV2640 gắn trên ESP32-S3-CAM, nhằm giải quyết vấn đề domain shift: model được
// train trên bộ ảnh Kaggle asl_alphabet_train (chụp bằng thiết bị khác) sẽ có phân phối
// pixel khác hẳn ảnh chụp thật từ OV2640.
//
// Sketch này KHÔNG phải kiến trúc cuối cùng — chỉ dùng trong Phase 3 để chạy
// pc/collect_calibration_set.py, sau đó chuyển hẳn sang esp32/edge_inference/edge_inference.ino.
//
// Cách dùng: nạp sketch này, mở Serial Monitor lấy IP, chạy
//   python pc/collect_calibration_set.py --esp32-ip <IP> --label A
// lần lượt cho từng chữ cái.

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
// Include chéo sang thư mục sketch khác để dùng chung pin/config — hoạt động với
// Arduino IDE/arduino-cli vì include tương đối được giải quyết theo đường dẫn file
// gốc trên đĩa. Nếu trình biên dịch bạn dùng không hỗ trợ, copy 2 file này vào
// cùng thư mục esp32/tools/ thay vì include tương đối.
#include "../edge_inference/camera_pins.h"
#include "../edge_inference/roi_config.h"

const char* ssid = "TEN_WIFI_CUA_BAN";
const char* password = "MAT_KHAU_WIFI";

WebServer server(80);

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE; // OV2640 trả ảnh xám ngay ở sensor, không cần tự convert
  config.frame_size = FRAMESIZE_QQVGA;       // 160x120, khớp CAM_FRAME_WIDTH/HEIGHT trong roi_config.h
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Camera init thất bại: 0x%x\n", err);
    while (true) delay(1000);
  }
}

void handleCapture() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }
  server.sendHeader("X-Width", String(fb->width));
  server.sendHeader("X-Height", String(fb->height));
  server.send_P(200, "application/octet-stream", (const char*)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);
  setupCamera();

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected: " + WiFi.localIP().toString());

  server.on("/capture", HTTP_GET, handleCapture);
  server.begin();
  Serial.println("📷 Capture server: GET /capture -> raw grayscale " +
                  String(CAM_FRAME_WIDTH) + "x" + String(CAM_FRAME_HEIGHT));
}

void loop() {
  server.handleClient();
}
