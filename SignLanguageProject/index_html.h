#pragma once

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <title>ESP32 Sign Language AI</title>
  <style>
    body { font-family: "Helvetica Neue", Arial, sans-serif; text-align: center; margin: 0; padding-top: 50px; background-color: #eef2f3; }
    h1 { color: #333; margin-bottom: 10px; }
    h3 { color: #666; font-weight: normal; margin-top: 0; }
    .card { background: white; padding: 40px; box-shadow: 0 10px 25px rgba(0,0,0,0.1); max-width: 400px; margin: auto; border-radius: 15px; }
    .result-box { background-color: #f8f9fa; border-radius: 10px; padding: 20px; margin: 20px 0; border: 2px solid #e9ecef; }
    .result { font-size: 100px; font-weight: bold; color: #007BFF; margin: 0; line-height: 1; }
    .label-text { font-size: 18px; color: #888; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 1px; }
    .conf { font-size: 24px; color: #28a745; font-weight: bold; }
    .footer { margin-top: 30px; font-size: 12px; color: #aaa; }
  </style>
</head>
<body>
  <div class="card">
    <h1>NHẬN DIỆN KÝ HIỆU</h1>
    <h3>Hệ thống AI nhúng trên ESP32-S3</h3>
    
    <div class="result-box">
      <div class="label-text">Ký hiệu nhận diện:</div>
      <div id="result" class="result">?</div>
    </div>
    
    <div>Độ tin cậy: <span id="confidence" class="conf">0%</span></div>
  </div>
  
  <div class="footer">Project I - HUST</div>

  <script>
    // Tự động cập nhật kết quả mỗi 300ms
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var json = JSON.parse(this.responseText);
          document.getElementById("result").innerHTML = json.label;
          document.getElementById("confidence").innerHTML = (json.score * 100).toFixed(1) + "%";
          
          // Đổi màu nếu độ tin cậy thấp
          if(json.score < 0.6) {
             document.getElementById("result").style.color = "#ccc";
             document.getElementById("confidence").style.color = "#ccc";
          } else {
             document.getElementById("result").style.color = "#007BFF";
             document.getElementById("confidence").style.color = "#28a745";
          }
        }
      };
      xhttp.open("GET", "/data", true);
      xhttp.send();
    }, 300); 
  </script>
</body>
</html>
)rawliteral";