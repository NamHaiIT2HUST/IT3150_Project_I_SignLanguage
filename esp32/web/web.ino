// File: web.ino
#include <WiFi.h>
#include <WebServer.h>
#include "web_interface.h" // <-- GỌI FILE HTML Ở ĐÂY

const char* ssid = "Repeater";
const char* password = "22222222";

WebServer server(80);
String currentLetter = "?"; 

// --- SETUP ---
void setup() {
  Serial.begin(115200);

  // 1. Kết nối WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ Connected: " + WiFi.localIP().toString());

  // 2. Route Trang chủ (Lấy từ file .h)
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html); 
  });

  // 3. Route Nhận dữ liệu từ PC
  server.on("/update", HTTP_GET, []() {
    if (server.hasArg("char")) {
      currentLetter = server.arg("char");
      server.send(200, "text/plain", "OK");
    } else server.send(400, "text/plain", "Err");
  });

  // 4. Route Gửi dữ liệu cho Web
  server.on("/get", HTTP_GET, []() {
    server.send(200, "text/plain", currentLetter);
  });

  server.begin();
  Serial.println("🚀 Web server started");
}

// --- LOOP ---
void loop() {
  server.handleClient();
}