#include <WiFi.h>
#include <WebServer.h>
#include "web_interface.h" 

const char* ssid = "Repeater";
const char* password = "22222222";

WebServer server(80);
String currentLetter = "?"; 

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ Connected: " + WiFi.localIP().toString());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html); 
  });

  server.on("/update", HTTP_GET, []() {
    if (server.hasArg("char")) {
      currentLetter = server.arg("char");
      server.send(200, "text/plain", "OK");
    } else server.send(400, "text/plain", "Err");
  });

  server.on("/get", HTTP_GET, []() {
    server.send(200, "text/plain", currentLetter);
  });

  server.begin();
  Serial.println("🚀 Web server started");
}

void loop() {
  server.handleClient();
}