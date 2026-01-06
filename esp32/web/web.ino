#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

const char* ssid = "Repeater";
const char* password = "22222222";

WebServer server(80);

// ================== HELPERS ==================
void sendFile(const char* path, const char* contentType) {
  if (!SPIFFS.exists(path)) {
    server.send(404, "text/plain", "File Not Found");
    return;
  }

  File file = SPIFFS.open(path, "r");
  server.streamFile(file, contentType);
  file.close();
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  // SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // Routes
  server.on("/", HTTP_GET, []() {
    sendFile("/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, []() {
    sendFile("/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, []() {
    sendFile("/script.js", "application/javascript");
  });

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}
