#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"
#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
//#include "tensorflow/lite/version.h"

#include "camera_pins.h"
#include "model_data.h"
#include "webpage.h"

const char* ssid = "TEN_WIFI_CUA_BAN";     
const char* password = "MAT_KHAU_WIFI";    

const char* CLASSES[] = {
  "0",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "A",
  "B",
  "C",
  "D",
  "Delete",
  "E",
  "F",
  "G",
  "H",
  "I",
  "J",
  "K",
  "L",
  "M",
  "N",
  "None",
  "O",
  "P",
  "Q",
  "R",
  "S",
  "Space",
  "T",
  "U",
  "V",
  "W",
  "X",
  "Y",
  "Z",
};
#define NUM_CLASSES 3
#define K_ARENA_SIZE 60000  
#define IMG_W 96
#define IMG_H 96

WebServer server(80);
String currentPred = "Waiting";
String currentConf = "0";

namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;
  uint8_t tensor_arena[K_ARENA_SIZE];
}

void initCamera() {
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE; //Ảnh xám
  config.frame_size = FRAMESIZE_96X96;       //Kích thước 96x96
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (psramFound()) {
    config.grab_mode = CAMERA_GRAB_LATEST;
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera Init Failed!");
    return;
  }
  Serial.println("Camera Ready!");
}

void setup() {
  Serial.begin(115200);
  
  //Kết nối WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi Connected: " + WiFi.localIP().toString());

  server.on("/", []() { server.send(200, "text/html", index_html); });
  server.on("/status", []() {
    String json = "{\"pred\":\"" + currentPred + "\", \"conf\":\"" + currentConf + "\"}";
    server.send(200, "application/json", json);
  });
  server.begin();

  initCamera();

  error_reporter = nullptr;
/*
  model = tflite::GetModel(model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model version mismatch!");
    return;
  }
*/
  static tflite::MicroMutableOpResolver<5> micro_op_resolver;
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddMaxPool2D();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddFullyConnected();
  micro_op_resolver.AddSoftmax();

  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, K_ARENA_SIZE, error_reporter);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    return;
  }

  input = interpreter->input(0);
  output = interpreter->output(0);
  Serial.println("AI System Ready!");
}

void loop() {
  server.handleClient(); 

  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) return;

  for (int i = 0; i < IMG_W * IMG_H; i++) {
    input->data.int8[i] = (int8_t)((int)fb->buf[i] - 128);
  }
  
  esp_camera_fb_return(fb);

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Invoke failed");
    return;
  }

  int8_t max_score = -128;
  int max_index = 0;
  
  for (int i = 0; i < NUM_CLASSES; i++) {
    if (output->data.int8[i] > max_score) {
      max_score = output->data.int8[i];
      max_index = i;
    }
  }

  float confidence = ((max_score + 128) / 255.0) * 100;

  currentPred = CLASSES[max_index];
  currentConf = String(confidence, 1);
  
  Serial.println(currentPred + " (" + currentConf + "%)");
  
  delay(100); 
}
