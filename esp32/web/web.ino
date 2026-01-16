#include <WiFi.h>      
#include <WebServer.h>  
#include "web_interface.h" 

const char* ssid = "NDNH";
const char* password = "23092005";

//Khởi tại Server chạy ở cổng
WebServer server(80);

//Biến để lưu ký tự nhận diện được
String currentLetter = "?"; 

void setup() {
  Serial.begin(115200); //Tốc độ ở cổng Serial

  WiFi.begin(ssid, password);
  Serial.print("Connecting"); //Hiển thị kết nối thành công

  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }

  Serial.println("\n✅ Connected: " + WiFi.localIP().toString());  //Hiện địa chỉ IP

  //Giao thức khi vào trình duyệt
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html); 
  });

  //Giao thức khi nhận yêu cầu
  server.on("/update", HTTP_GET, []() {
    if (server.hasArg("char")) {
      currentLetter = server.arg("char"); 
      server.send(200, "text/plain", "OK"); 
    } else {
      server.send(400, "text/plain", "Err");
    }
  });

  //Giao thức khi trả kết quả thu được
  server.on("/get", HTTP_GET, []() {
    server.send(200, "text/plain", currentLetter); 
  });

  server.begin();
  Serial.println("🚀 Web server started");
}

void loop() {
  server.handleClient();
}