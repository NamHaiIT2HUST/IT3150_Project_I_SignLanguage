#include <WiFi.h>
#include <WebServer.h>

#include "webpage_test.h"

const char* ssid = "Tang3";      
const char* password = "23092005";    

WebServer server(80);
String currentPred = "None";
String currentConf = "0";

void setup() {
  Serial.begin(115200);

  //Kết nối Wifi
  Serial.print("Dang ket noi Wifi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("");
  Serial.println("Da ket noi Wifi!");
  Serial.print("Dia chi IP cua Web: http://");
  Serial.println(WiFi.localIP());

  //Cài đặt trang Web
  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });

  //Cài đặt API lấy dữ liệu (/status)
  server.on("/status", []() {
    String json = "{\"pred\":\"" + currentPred + "\", \"conf\":\"" + currentConf + "\"}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("Web Server da san sang!");
}

void loop() {
  server.handleClient(); 

  static unsigned long lastTime = 0;
  if (millis() - lastTime > 3000) {
    lastTime = millis();

    int randNumber = random(3);
    
    if (randNumber == 0) {
      currentPred = "A";
      currentConf = String(random(85, 100)); 
      Serial.println("AI gia lap: Phat hien chu A");
    } 
    else if (randNumber == 1) {
      currentPred = "B";
      currentConf = String(random(85, 100));
      Serial.println("AI gia lap: Phat hien chu B");
    } 
    else {
      currentPred = "None";
      currentConf = "15";
      Serial.println("AI gia lap: Khong thay gi");
    }
  }
}
