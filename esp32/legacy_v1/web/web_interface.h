const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Hệ thống nhận diện ngôn ngữ ký hiệu</title>
  <link href="https://fonts.googleapis.com/css2?family=Segoe+UI:wght@400;600;700&display=swap" rel="stylesheet">
  <style>
    /* ... (Giữ nguyên phần Style như cũ) ... */
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      height: 100vh;
      overflow: hidden; 
      padding: 10px; 
      display: flex;
      justify-content: center;
      align-items: center;
    }
    .container {
      width: 100%;
      max-width: 1400px;
      height: 100%;
      background: rgba(255, 255, 255, 0.9);
      backdrop-filter: blur(20px);
      border-radius: 15px;
      padding: 15px;
      box-shadow: 0 20px 40px rgba(0, 0, 0, 0.2);
      border: 1px solid rgba(255, 255, 255, 0.3);
      display: flex;
      flex-direction: column;
      overflow: hidden; 
    }
    h1 {
      text-align: center;
      font-size: 1.5rem; 
      font-weight: 700;
      margin-bottom: 10px;
      background: linear-gradient(45deg, #667eea, #764ba2);
      -webkit-background-clip: text; -webkit-text-fill-color: transparent;
      text-transform: uppercase;
      flex-shrink: 0;
    }
    .content-grid {
      display: grid;
      grid-template-columns: 1.8fr 1fr;
      gap: 15px;
      flex: 1; 
      min-height: 0; 
      overflow: hidden;
    }
    .left-panel {
      display: flex;
      flex-direction: column;
      gap: 10px;
      height: 100%;
      min-height: 0; 
      overflow: hidden;
    }
    .card {
      background: rgba(255, 255, 255, 0.6);
      border-radius: 12px;
      padding: 8px;
      border: 1px solid rgba(255, 255, 255, 0.5);
      display: flex; 
      flex-direction: column;
      min-height: 0; 
    }
    .card-title {
      font-size: 0.85rem; font-weight: 700; color: #555;
      margin-bottom: 5px; text-transform: uppercase;
      display: flex; justify-content: space-between; align-items: center;
      flex-shrink: 0;
    }
    .camera-section {
      flex: 1.2; 
      display: flex; flex-direction: column;
    }
    .cheatsheet-section {
      flex: 0.8;
      display: flex; flex-direction: column;
    }
    .video-wrapper, .cheat-sheet-wrapper {
      flex: 1; 
      width: 100%; 
      height: 100%;
      border-radius: 8px; 
      overflow: hidden;
      display: flex; 
      align-items: center; 
      justify-content: center;
      background: #000; 
      min-height: 0;
    }
    img#video, .cheat-sheet-img { 
      width: 100%; 
      height: 100%; 
      object-fit: contain; 
      display: block; 
    }
    .right-panel {
      background: rgba(255, 255, 255, 0.5);
      border-radius: 12px;
      padding: 15px;
      display: flex; flex-direction: column;
      align-items: center;
      justify-content: space-between;
      border: 1px solid rgba(255, 255, 255, 0.5);
      height: 100%;
      min-height: 0;
      overflow: hidden;
    }
    .timer-ring { 
      position: relative; width: 80px; height: 80px; 
      flex-shrink: 0; margin-bottom: 10px;
    }
    .timer-svg { transform: rotate(-90deg); width: 100%; height: 100%; }
    .timer-bg { fill: none; stroke: #ddd; stroke-width: 7; }
    .timer-fg { 
      fill: none; stroke: #667eea; stroke-width: 7; stroke-linecap: round; 
      transition: stroke-dashoffset 1s linear;
    }
    .timer-text {
      position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%);
      font-size: 1.8rem; font-weight: 700; color: #444;
    }
    .result-area { text-align: center; flex-shrink: 0; margin-bottom: 5px; }
    .label-small { font-size: 0.75rem; color: #777; font-weight: 600; text-transform: uppercase; }
    .big-letter {
      font-size: 4rem; font-weight: 800; line-height: 1;
      background: linear-gradient(45deg, #667eea, #764ba2);
      -webkit-background-clip: text; -webkit-text-fill-color: transparent;
    }
    .history-box {
      width: 100%;
      flex: 1; 
      min-height: 0; 
      margin: 10px 0;
      background: rgba(255, 255, 255, 0.8);
      border-radius: 10px; padding: 10px;
      border-left: 4px solid #667eea;
      font-family: 'Courier New', monospace; font-size: 1rem; color: #333;
      overflow-y: auto; 
    }
    .button-group { 
      display: flex; gap: 8px; width: 100%; 
      flex-shrink: 0; 
      margin-top: 5px;
    }
    button {
      flex: 1; padding: 10px; border: none; border-radius: 10px;
      font-size: 0.85rem; font-weight: 600; cursor: pointer;
      transition: all 0.3s ease; position: relative; overflow: hidden;
      color: white; box-shadow: 0 4px 10px rgba(0,0,0,0.15);
      white-space: nowrap;
    }
    button:hover { transform: translateY(-2px); }
    .btn-start { background: linear-gradient(45deg, #4CAF50, #45a049); }
    .btn-stop { background: linear-gradient(45deg, #f44336, #da190b); display: none; }
    .btn-clear { background: linear-gradient(45deg, #FF9800, #e68900); flex: 0.6; }
    .badge-live {
      background: #f44336; color: white; padding: 2px 6px;
      border-radius: 6px; font-size: 0.65rem; animation: pulse 1.5s infinite;
      vertical-align: middle;
    }
    @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.5; } 100% { opacity: 1; } }
    @media (max-width: 800px) {
      body { height: auto; overflow: auto; display: block; }
      .container { height: auto; overflow: visible; }
      .content-grid { grid-template-columns: 1fr; overflow: visible; }
      .left-panel { height: auto; }
      .camera-section, .cheatsheet-section { height: 300px; flex: none; }
      .right-panel { height: auto; }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Hệ thống nhận dạng ngôn ngữ ký hiệu</h1>
    <div class="content-grid">
      <div class="left-panel">
        <div class="card camera-section">
          <div class="card-title">
            <span>📹 Camera</span>
            <span class="badge-live">Live</span>
          </div>
          <div class="video-wrapper">
            <img id="video" src="http://172.20.10.5:5000/video" alt="Đang kết nối camera...">
          </div>
        </div>
        <div class="card cheatsheet-section">
          <div class="card-title">📖 Bảng Ký Hiệu (A-Z)</div>
          <div class="cheat-sheet-wrapper">
            <img src="http://172.20.10.5:5000/static/asl_table_left_hand.jpg" class="cheat-sheet-img" alt="Bảng mẫu">
          </div>
        </div>
      </div>
      <div class="right-panel">
        <div class="timer-ring">
          <svg class="timer-svg" viewBox="0 0 100 100">
            <circle class="timer-bg" cx="50" cy="50" r="45"></circle>
            <circle class="timer-fg" cx="50" cy="50" r="45" stroke-dasharray="282.7" stroke-dashoffset="0"></circle>
          </svg>
          <div class="timer-text" id="timer-display">5</div>
        </div>
        <div class="result-area">
          <div class="label-small">Ký Tự Nhận Diện</div>
          <div class="big-letter" id="live-char">--</div>
        </div>
        <div class="history-box" id="history-box">
          <span id="history-text"></span><span style="animation:pulse 1s infinite; font-weight:bold;">|</span>
        </div>
        <div class="button-group">
          <button id="btn-start" class="btn-start" onclick="startSystem()">▶ Start</button>
          <button id="btn-stop" class="btn-stop" onclick="stopSystem()">⏹ Stop</button>
          <button class="btn-clear" onclick="clearHistory()">🗑 Xóa</button>
        </div>
      </div>
    </div>
  </div>
  <script>
    let timeLeft = 5; let maxTime = 5; let isRunning = false;
    let currentDetect = ""; let historyStr = "";
    const circle = document.querySelector('.timer-fg');
    const circumference = 282.7;
    circle.style.strokeDasharray = `${circumference} ${circumference}`;
    function setProgress(percent) {
      const offset = circumference - (percent / 100) * circumference;
      circle.style.strokeDashoffset = offset;
    }
    function startSystem() {
      isRunning = true;
      document.getElementById("btn-start").style.display = "none";
      document.getElementById("btn-stop").style.display = "block";
      timeLeft = 5; setProgress(100);
      document.querySelector('.timer-fg').style.stroke = "#f44336"; 
    }
    function stopSystem() {
      isRunning = false;
      document.getElementById("btn-start").style.display = "block";
      document.getElementById("btn-stop").style.display = "none";
      document.getElementById("timer-display").innerText = "5"; setProgress(0);
      document.querySelector('.timer-fg').style.stroke = "#667eea";
      document.getElementById("live-char").innerText = "--";
    }
    setInterval(() => {
      fetch('/get').then(res => res.text()).then(data => {
        currentDetect = data;
        const charEl = document.getElementById("live-char");
        if(isRunning) {
           charEl.innerText = (data === "?" || data === "") ? "--" : data;
        }
      }).catch(e => console.log("Waiting..."));
    }, 300);
    setInterval(() => {
      if (!isRunning) return;
      timeLeft--;
      document.getElementById("timer-display").innerText = timeLeft;
      setProgress((timeLeft / maxTime) * 100);
      if (timeLeft <= 0) {
        if (currentDetect !== "?" && currentDetect !== "" && currentDetect !== "--") {
          historyStr += currentDetect;
          document.getElementById("history-text").innerText = historyStr;
          const box = document.getElementById("history-box");
          box.scrollTop = box.scrollHeight;
        }
        timeLeft = 5; setProgress(100);
      }
    }, 1000);
    function clearHistory() { 
      historyStr = ""; 
      document.getElementById("history-text").innerText = ""; 
    }
  </script>
</body>
</html>
)rawliteral";