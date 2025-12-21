const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="utf-8">
  <title>ESP32 Sign Language</title>
  <style>
    body { font-family: 'Segoe UI', sans-serif; text-align: center; background: #f4f7f6; margin: 0; padding: 0; }
    .header { background-color: #007bff; color: white; padding: 15px; margin-bottom: 20px; }
    h1 { margin: 0; font-size: 24px; }
    .card { background: white; max-width: 350px; margin: auto; padding: 25px; border-radius: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }
    .result-box { font-size: 100px; font-weight: bold; color: #333; margin: 10px 0; min-height: 120px; line-height: 120px;}
    .label { color: #888; font-size: 14px; text-transform: uppercase; letter-spacing: 1px; }
    .conf-bar { width: 100%; background: #e9ecef; height: 10px; border-radius: 5px; margin-top: 10px; overflow: hidden; }
    .conf-fill { height: 100%; background: #28a745; width: 0%; transition: width 0.3s; }
    .conf-text { color: #28a745; font-weight: bold; margin-top: 5px; font-size: 14px; }
  </style>
</head>
<body>
  <div class="header">
    <h1>Hệ Thống Nhận Diện Ký Hiệu</h1>
  </div>

  <div class="card">
    <div class="label">Kết quả nhận diện</div>
    <div class="result-box" id="result">...</div>
    
    <div class="label">Độ tin cậy</div>
    <div class="conf-bar"><div class="conf-fill" id="bar"></div></div>
    <div class="conf-text" id="conf">0%</div>
  </div>

  <script>
    setInterval(function() {
      fetch("/status")
        .then(response => response.json())
        .then(data => {
          document.getElementById("result").innerText = data.pred;
          document.getElementById("conf").innerText = data.conf + "%";
          document.getElementById("bar").style.width = data.conf + "%";
          
          let color = (data.conf > 70) ? "#28a745" : "#ffc107";
          document.getElementById("bar").style.backgroundColor = color;
        })
        .catch(e => console.log(e));
    }, 500);
  </script>
</body>
</html>
)rawliteral";
