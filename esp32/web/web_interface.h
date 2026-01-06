// File: web_interface.h
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>AI Sign Language Hub</title>
  <link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&family=Roboto+Mono:wght@400;700&display=swap" rel="stylesheet">
  <style>
    :root {
      --primary: #00f3ff;
      --secondary: #bc13fe;
      --bg-dark: #050510;
      --glass: rgba(255, 255, 255, 0.05);
      --border: rgba(255, 255, 255, 0.1);
    }
    body {
      background: radial-gradient(circle at center, #1a1a2e, #000);
      color: white; font-family: 'Roboto Mono', monospace;
      margin: 0; min-height: 100vh;
      display: flex; flex-direction: column; align-items: center; justify-content: center;
    }
    h1 {
      font-family: 'Orbitron', sans-serif; font-size: 2.5rem; text-transform: uppercase; letter-spacing: 3px;
      margin-bottom: 30px; text-shadow: 0 0 10px var(--primary), 0 0 20px var(--secondary);
      background: -webkit-linear-gradient(#00f3ff, #bc13fe); -webkit-background-clip: text; -webkit-text-fill-color: transparent;
    }
    .dashboard {
      display: grid; grid-template-columns: 1.5fr 1fr; gap: 30px;
      width: 90%; max-width: 1200px; padding: 20px;
      background: var(--glass); backdrop-filter: blur(15px);
      border-radius: 20px; border: 1px solid var(--border); box-shadow: 0 0 50px rgba(0, 243, 255, 0.1);
    }
    .video-box {
      position: relative; border-radius: 15px; overflow: hidden;
      border: 2px solid var(--primary); box-shadow: 0 0 20px rgba(0, 243, 255, 0.2);
    }
    img#video { width: 100%; height: 100%; object-fit: cover; display: block; }
    .live-badge {
      position: absolute; top: 15px; left: 15px; background: rgba(255, 0, 0, 0.7);
      padding: 5px 10px; border-radius: 5px; font-weight: bold; font-size: 0.8rem; animation: pulse 1.5s infinite;
    }
    .info-panel { display: flex; flex-direction: column; gap: 20px; }
    .card {
      background: rgba(0, 0, 0, 0.4); border-radius: 15px; padding: 20px;
      border: 1px solid var(--border); text-align: center; position: relative;
    }
    .card-title { color: #aaa; font-size: 0.9rem; text-transform: uppercase; margin-bottom: 10px; letter-spacing: 1px; }
    .big-char {
      font-size: 5rem; font-weight: bold; font-family: 'Orbitron', sans-serif;
      color: var(--primary); text-shadow: 0 0 30px var(--primary); transition: all 0.3s ease;
    }
    .timer-container { position: absolute; top: 10px; right: 10px; width: 40px; height: 40px; }
    .progress-ring__circle { transition: stroke-dashoffset 0.35s; transform: rotate(-90deg); transform-origin: 50% 50%; }
    .history-box {
      flex-grow: 1; background: #000; border: 1px solid #333; border-radius: 10px; padding: 15px;
      font-family: 'Courier New', monospace; overflow: hidden; display: flex; flex-direction: column;
    }
    #history-text {
      color: var(--secondary); font-size: 1.5rem; word-break: break-all;
      text-shadow: 0 0 5px var(--secondary); line-height: 1.5;
    }
    .cursor { display: inline-block; width: 10px; height: 20px; background: var(--secondary); animation: blink 1s infinite; }
    .btn-reset {
      background: transparent; border: 1px solid var(--primary); color: var(--primary);
      padding: 10px; border-radius: 8px; cursor: pointer; font-family: 'Orbitron', sans-serif;
      font-weight: bold; transition: 0.3s; text-transform: uppercase;
    }
    .btn-reset:hover { background: var(--primary); color: black; box-shadow: 0 0 15px var(--primary); }
    @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.5; } 100% { opacity: 1; } }
    @keyframes blink { 0%, 100% { opacity: 1; } 50% { opacity: 0; } }
    @media (max-width: 768px) { .dashboard { grid-template-columns: 1fr; } }
  </style>
</head>
<body>
  <h1>Sign Language AI</h1>
  <div class="dashboard">
    <div class="video-box">
      <div class="live-badge">● LIVE FEED</div>
      
      <img id="video" src="http://192.168.1.70:5000/video">
      
    </div>
    <div class="info-panel">
      <div class="card">
        <div class="timer-container">
          <svg class="progress-ring" width="40" height="40">
            <circle class="progress-ring__circle" stroke="yellow" stroke-width="4" fill="transparent" r="16" cx="20" cy="20"/>
          </svg>
          <div style="position:absolute; top:50%; left:50%; transform:translate(-50%, -50%); font-size:12px; font-weight:bold; color:yellow;" id="timer-text">5</div>
        </div>
        <div class="card-title">Realtime Detection</div>
        <div class="big-char" id="live-char">?</div>
      </div>
      <div class="history-box">
        <div class="card-title" style="text-align:left; border-bottom:1px solid #333; padding-bottom:5px;">> SYSTEM_LOG</div>
        <div style="margin-top:10px;">
          <span id="history-text"></span><span class="cursor"></span>
        </div>
      </div>
      <button class="btn-reset" onclick="clearHistory()">RESET DATA</button>
    </div>
  </div>
  <script>
    let timeLeft = 5; let maxTime = 5; let currentDetect = "?"; let historyStr = "";
    const circle = document.querySelector('.progress-ring__circle');
    const radius = circle.r.baseVal.value; const circumference = radius * 2 * Math.PI;
    circle.style.strokeDasharray = `${circumference} ${circumference}`; circle.style.strokeDashoffset = circumference;
    function setProgress(percent) { const offset = circumference - (percent / 100) * circumference; circle.style.strokeDashoffset = offset; }
    
    setInterval(() => {
      fetch('/get').then(res => res.text()).then(data => {
        currentDetect = data; document.getElementById("live-char").innerText = data;
        const charEl = document.getElementById("live-char");
        if(data !== "?") charEl.style.textShadow = "0 0 50px #00f3ff"; else charEl.style.textShadow = "none";
      }).catch(e => console.log("Waiting..."));
    }, 300);

    setInterval(() => {
      timeLeft--;
      let percent = (timeLeft / maxTime) * 100; setProgress(percent);
      document.getElementById("timer-text").innerText = timeLeft;
      if (timeLeft <= 0) {
        if (currentDetect !== "?" && currentDetect !== "") {
          historyStr += currentDetect; document.getElementById("history-text").innerText = historyStr;
        }
        timeLeft = 5; setProgress(100);
      }
    }, 1000);
    function clearHistory() { historyStr = ""; document.getElementById("history-text").innerText = ""; }
  </script>
</body>
</html>
)rawliteral";