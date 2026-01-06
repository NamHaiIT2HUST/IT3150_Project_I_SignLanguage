#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Repeater";
const char* password = "22222222";

WebServer server(80);

String currentChar = "-";

// Trang web chính
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='utf-8'>";
  html += "<meta http-equiv='refresh' content='1'>";
  html += "<title>Sign Language</title>";
  html += "</head><body>";
  html += "<h1>ESP32-S3 Sign Language</h1>";
  html += "<h2>Detected character:</h2>";
  html += "<h1 style='color:blue; font-size:80px;'>" + currentChar + "</h1>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Nhận ký tự từ PC
void handleUpdate() {
  if (server.hasArg("char")) {
    currentChar = server.arg("char");
    Serial.println("Received: " + currentChar);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing char");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.begin();

  Serial.println("WebServer started");
}

void loop() {
  server.handleClient();
}
