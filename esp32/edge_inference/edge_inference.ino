// Firmware v2: ESP32-S3-CAM tự chụp ảnh, tự tiền xử lý và tự suy luận CNN
// (TensorFlow Lite Micro, int8) ngay trên chip — KHÔNG host web server. Chỉ gửi
// kết quả (JSON nhỏ) lên PC server trung tâm (pc/server.py) qua HTTP POST.
//
// Thư viện cần cài (Arduino Library Manager hoặc PlatformIO):
//   - esp-tflite-micro (Espressif) hoặc Chirale_TensorFlowLite
//   - esp32-camera (đi kèm ESP32 Arduino core khi chọn board có camera, vd
//     "ESP32S3 Dev Module" với PSRAM bật)
// API của thư viện TFLite Micro có thể khác nhau giữa các phiên bản — nếu lỗi
// biên dịch, đối chiếu sketch ví dụ "hello_world"/"person_detection" đi kèm thư viện.
//
// CHƯA kiểm thử trên phần cứng thật (chưa có board ESP32-S3-CAM lúc viết code này).
// Trước khi coi là hoàn thiện, cần trên board thật:
//   1. Xác nhận lại camera_pins.h đúng với board Freenove thực tế đang dùng.
//   2. Đọc interpreter->arena_used_bytes() qua Serial để tinh chỉnh kTensorArenaSize.
//   3. Đo FPS thực tế (đã log sẵn mỗi frame bên dưới) và ghi vào báo cáo.
//   4. model_data.h phải được sinh từ model đã train+quantize thật (pc/train_model.py
//      -> pc/convert_to_c_array.py), không dùng file placeholder.

#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

#include "camera_pins.h"
#include "roi_config.h"
#include "model_data.h"

#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>

// ---- Cấu hình mạng ----
const char* ssid = "TEN_WIFI_CUA_BAN";
const char* password = "MAT_KHAU_WIFI";
const char* PC_SERVER_URL = "http://192.168.1.100:5000/predict"; // đổi theo IP PC thật

// ---- Nhãn: PHẢI khớp đúng thứ tự trong model/labels.json ----
const char* LABELS[] = {
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M", "N",
  "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y"
};
const int NUM_CLASSES = sizeof(LABELS) / sizeof(LABELS[0]);
const float CONFIDENCE_THRESHOLD = 0.7f;

// ---- TensorFlow Lite Micro ----
namespace {
tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;
const tflite::Model* tf_model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Ước tính ban đầu cho kiến trúc CNN gọn (16/32/32 filters, input 48x48) — BẮT BUỘC
// đo interpreter->arena_used_bytes() thực tế trên board rồi tinh chỉnh lại con số này.
constexpr int kTensorArenaSize = 80 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

bool setupModel() {
  tf_model = tflite::GetModel(g_model_data);
  if (tf_model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema version khong khop voi thu vien TFLite Micro dang dung");
    return false;
  }

  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      tf_model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors() that bai - tensor arena co the qua nho");
    return false;
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.printf("Model san sang. Tensor arena dung: %u / %d bytes\n",
                (unsigned)interpreter->arena_used_bytes(), kTensorArenaSize);
  return true;
}

bool setupCamera() {
  camera_config_t config = {};
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
  config.pixel_format = PIXFORMAT_GRAYSCALE; // OV2640 tra anh xam ngay o sensor
  config.frame_size = FRAMESIZE_QQVGA;       // 160x120, khop CAM_FRAME_WIDTH/HEIGHT
  config.fb_count = 1;

  return esp_camera_init(&config) == ESP_OK;
}

// Cat vung vuong o giua khung hinh roi resize nearest-neighbor xuong input model.
// Khong dung OpenCV (khong co tren MCU) nen tu viet resize don gian.
// Quantize dua tren scale/zero_point THAT cua tensor input (khong hardcode), vi
// gia tri nay do TFLiteConverter tinh tu representative_dataset luc train, co the
// khac nhau giua cac lan train.
void preprocessToInput(camera_fb_t* fb) {
  const int x0 = (fb->width - ROI_SIZE) / 2;
  const int y0 = (fb->height - ROI_SIZE) / 2;
  const float scale = input->params.scale;
  const int zero_point = input->params.zero_point;

  for (int y = 0; y < MODEL_INPUT_SIZE; y++) {
    int src_y = y0 + y * ROI_SIZE / MODEL_INPUT_SIZE;
    for (int x = 0; x < MODEL_INPUT_SIZE; x++) {
      int src_x = x0 + x * ROI_SIZE / MODEL_INPUT_SIZE;
      uint8_t pixel = fb->buf[src_y * fb->width + src_x];
      float normalized = pixel / 255.0f;  // khop voi gray/255.0 luc train
      int32_t quantized = (int32_t)lroundf(normalized / scale) + zero_point;
      if (quantized < -128) quantized = -128;
      if (quantized > 127) quantized = 127;
      input->data.int8[y * MODEL_INPUT_SIZE + x] = (int8_t)quantized;
    }
  }
}

void sendResult(const char* label, float confidence) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(PC_SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  char body[96];
  snprintf(body, sizeof(body), "{\"char\":\"%s\",\"confidence\":%.2f}", label, confidence);
  int code = http.POST((uint8_t*)body, strlen(body));
  if (code > 0) {
    Serial.printf("Gui '%s' (%.2f) -> HTTP %d\n", label, confidence, code);
  }
  http.end();
}

void setup() {
  Serial.begin(115200);

  if (!setupCamera()) {
    Serial.println("Camera init that bai");
    while (true) delay(1000);
  }
  if (!setupModel()) {
    while (true) delay(1000);
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected: " + WiFi.localIP().toString());
}

void loop() {
  unsigned long t0 = millis();

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    delay(50);
    return;
  }

  preprocessToInput(fb);
  esp_camera_fb_return(fb);

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Invoke() that bai");
    return;
  }

  const float out_scale = output->params.scale;
  const int out_zero_point = output->params.zero_point;

  int best_idx = 0;
  int8_t best_raw = output->data.int8[0];
  for (int i = 1; i < NUM_CLASSES; i++) {
    if (output->data.int8[i] > best_raw) {
      best_raw = output->data.int8[i];
      best_idx = i;
    }
  }
  float confidence = (best_raw - out_zero_point) * out_scale;

  unsigned long frame_ms = millis() - t0;
  Serial.printf("Frame: %lums (%.1f FPS) -> %s (%.2f)\n",
                frame_ms, 1000.0f / (float)max(frame_ms, 1UL),
                LABELS[best_idx], confidence);

  if (confidence > CONFIDENCE_THRESHOLD) {
    sendResult(LABELS[best_idx], confidence);
  }
}
