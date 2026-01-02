#define EI_CLASSIFIER_ALLOCATION_HEAP 1

const char* ssid = "Tang3";   
const char* password = "23092005";  

#include <SignLanguage_ESP32_inferencing.h>

#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"
#include "camera_pins.h"
#include "index_html.h"

WebServer server(80);

String currentLabel = "-";
float currentScore = 0.0;
camera_fb_t *fb = NULL;

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    size_t pixel_ix = offset;
    for (size_t i = 0; i < length; i++) {
        if (fb) {
            out_ptr[i] = (float)fb->buf[pixel_ix];
        } else {
            out_ptr[i] = 0.0;
        }
        pixel_ix++;
    }
    return 0;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nKhoi dong he thong...");

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

  config.frame_size = FRAMESIZE_96X96;      
  config.pixel_format = PIXFORMAT_GRAYSCALE; 
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  Serial.println("Camera Init Success!");

  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("Dia chi IP Web: http://");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/data", HTTP_GET, []() {
    String json = "{\"label\":\"" + currentLabel + "\", \"score\":" + String(currentScore) + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  server.handleClient(); 

  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  signal_t signal;
  signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
  signal.get_data = &raw_feature_get_data;

  ei_impulse_result_t result = { 0 };
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

  if (res == EI_IMPULSE_OK) {
    float max_score = 0.0;
    String max_label = "Unknown";

    for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
      if (result.classification[i].value > max_score) {
        max_score = result.classification[i].value;
        max_label = result.classification[i].label;
      }
    }

    currentScore = max_score;
    if (max_score > 0.65) { 
       currentLabel = max_label;
       Serial.printf("Ket qua: %s (%.1f%%)\n", max_label.c_str(), max_score * 100);
    } else {
       currentLabel = "...";
    }
  } else {
    Serial.printf("AI Error: %d\n", res);
  }

  esp_camera_fb_return(fb);
  fb = NULL;

  delay(50); 
}