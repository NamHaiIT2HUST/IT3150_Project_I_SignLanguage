const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="utf-8">
  <title>SignLanguage System</title>
  <link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&family=Roboto+Mono:wght@400;700&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg-color: #0d1117;
      --panel-color: #161b22;
      --accent-color: #00f0ff; /* Màu xanh Neon */
      --text-color: #c9d1d9;
      --danger-color: #ff4b4b;
      --success-color: #00e676;
    }

    * { box-sizing: border-box; }
    body { 
      margin: 0; padding: 0; 
      font-family: 'Roboto Mono', monospace; 
      background-color: var(--bg-color); 
      color: var(--text-color);
      display: flex; height: 100vh; overflow: hidden;
    }

    /* --- CỘT TRÁI: CAMERA SCANNER --- */
    .left-panel {
      flex: 6; 
      background: #000;
      position: relative;
      border-right: 2px solid #30363d;
      display: flex; align-items: center; justify-content: center;
      overflow: hidden;
    }
    
    .cam-view { width: 100%; height: 100%; object-fit: contain; display: none; }

    /* Hiệu ứng Scanning khi chưa có Cam */
    .scanner-overlay {
      position: absolute; width: 100%; height: 100%;
      background: linear-gradient(to bottom, transparent 50%, rgba(0, 240, 255, 0.1) 51%, transparent 52%);
      background-size: 100% 200%;
      animation: scan 3s linear infinite;
      z-index: 2;
      pointer-events: none;
    }
    .cam-placeholder {
      color: var(--accent-color);
      text-align: center; border: 1px dashed var(--accent-color);
      padding: 40px; border-radius: 10px;
      box-shadow: 0 0 15px rgba(0, 240, 255, 0.2);
    }
    @keyframes scan { 0% {background-position: 0% 0%;} 100% {background-position: 0% 100%;} }

    /* --- CỘT PHẢI: CONTROL PANEL --- */
    .right-panel {
      flex: 4; 
      background-color: var(--panel-color);
      padding: 25px;
      display: flex; flex-direction: column;
      box-shadow: -5px 0 20px rgba(0,0,0,0.5);
    }

    .header {
      display: flex; justify-content: space-between; align-items: center;
      border-bottom: 1px solid #30363d; padding-bottom: 15px; margin-bottom: 20px;
    }
    h2 { margin: 0; font-family: 'Orbitron', sans-serif; color: var(--accent-color); letter-spacing: 2px; }
    
    .status-dot {
      height: 12px; width: 12px; background-color: var(--success-color);
      border-radius: 50%; display: inline-block;
      box-shadow: 0 0 10px var(--success-color);
      animation: pulse 2s infinite;
    }

    /* Ô hiển thị kết quả AI */
    .live-box {
      background: #0d1117; border: 1px solid #30363d;
      border-radius: 12px; padding: 20px; text-align: center;
      margin-bottom: 20px; position: relative;
    }
    .big-char { 
      font-family: 'Orbitron', sans-serif; font-size: 90px; 
      color: #fff; text-shadow: 0 0 20px rgba(255,255,255,0.5);
      margin: 10px 0; line-height: 1;
    }
    .conf-bar-bg { width: 100%; height: 6px; background: #30363d; border-radius: 3px; margin-top: 10px; }
    .conf-bar-fill { height: 100%; background: var(--accent-color); width: 0%; border-radius: 3px; transition: width 0.3s; box-shadow: 0 0 10px var(--accent-color); }

    /* Ô Văn bản */
    .text-area {
      flex-grow: 1; background: #0d1117;
      border: 1px solid #30363d; border-radius: 8px;
      padding: 15px; font-size: 20px; color: #fff;
      font-family: 'Roboto Mono', monospace;
      margin-bottom: 20px; overflow-y: auto;
      white-space: pre-wrap;
    }
    .cursor { display: inline-block; width: 10px; height: 20px; background: var(--accent-color); animation: blink 1s infinite; vertical-align: bottom;}

    /* Terminal Log (Lịch sử) */
    .log-box {
      height: 100px; background: black; color: #00ff00;
      font-size: 12px; padding: 10px; overflow-y: hidden;
      border-top: 2px solid #333; margin-bottom: 20px;
      font-family: 'Courier New', Courier, monospace;
    }

    /* Nút bấm */
    .controls { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 10px; }
    .btn {
      padding: 12px; border: none; border-radius: 5px;
      font-weight: bold; cursor: pointer; color: #0d1117;
      text-transform: uppercase; font-family: 'Orbitron', sans-serif;
      transition: all 0.2s;
    }
    .btn-space { background: var(--accent-color); }
    .btn-clear { background: var(--danger-color); color: white; }
    .btn-speech { background: #fff; }
    .btn:hover { opacity: 0.8; transform: translateY(-2px); }

    @keyframes pulse { 0% {opacity: 1;} 50% {opacity: 0.5;} 100% {opacity: 1;} }
    @keyframes blink { 0%, 100% {opacity: 1;} 50% {opacity: 0;} }
    
    /* Mobile Responsive */
    @media (max-width: 768px) {
      body { flex-direction: column; height: auto; overflow: auto; }
      .left-panel { height: 300px; flex: none; width: 100%; border-right: none; border-bottom: 2px solid #30363d;}
      .right-panel { width: 100%; flex: none; }
    }
  </style>
</head>
<body>

  <div class="left-panel">
    <div class="scanner-overlay"></div> <img id="stream" src="" class="cam-view">
    <div class="cam-placeholder">
      <div style="font-size: 40px;">📷</div>
      <div>SYSTEM INITIALIZING...</div>
      <div style="font-size: 12px; margin-top:10px; color:#666;">WAITING FOR VIDEO FEED</div>
    </div>
  </div>

  <div class="right-panel">
    <div class="header">
      <h2>SIGN LANGUAGE SYSTEM</h2>
      <div style="display:flex; align-items:center; gap:10px;">
        <span style="font-size:12px; color:#8b949e;">ONLINE</span>
        <span class="status-dot"></span>
      </div>
    </div>

    <div class="live-box">
      <div style="font-size: 12px; color: #8b949e; letter-spacing: 1px;">DETECTED GESTURE</div>
      <div class="big-char" id="liveChar">--</div>
      <div style="font-size: 12px; color: var(--accent-color);">CONFIDENCE: <span id="conf">0</span>%</div>
      <div class="conf-bar-bg"><div class="conf-bar-fill" id="bar"></div></div>
    </div>

    <div class="log-box" id="logBox">
      <div>> System started...</div>
      <div>> Connecting to ESP32-S3 AI Core...</div>
      <div>> Ready.</div>
    </div>

    <div style="margin-bottom: 5px; font-size: 12px; color: #8b949e;">OUTPUT TEXT:</div>
    <div class="text-area" id="sentenceBoard">
      <span id="textContent"></span><span class="cursor"></span>
    </div>

    <div class="controls">
      <button class="btn btn-space" onclick="addSpace()">SPACE [_]</button>
      <button class="btn btn-speech" onclick="toggleSpeech()" id="btnSpeech">🔊 ON</button>
      <button class="btn btn-clear" onclick="clearText()">CLEAR [X]</button>
    </div>
  </div>

  <script>
    let sentence = "";
    let lastState = "None";
    let isSpeechOn = true;
    const logBox = document.getElementById("logBox");

    function updateBoard() {
      document.getElementById("textContent").innerText = sentence;
    }

    function addSpace() { 
      sentence += " "; updateBoard(); 
      addLog("User input: SPACE");
    }
    
    function clearText() { 
      sentence = ""; updateBoard(); 
      addLog("User input: CLEAR DATA");
    }

    function toggleSpeech() {
      isSpeechOn = !isSpeechOn;
      document.getElementById("btnSpeech").innerText = isSpeechOn ? "🔊 ON" : "🔇 OFF";
      addLog("System: Speech " + (isSpeechOn ? "Enabled" : "Disabled"));
    }

    // Hàm thêm dòng log kiểu hacker
    function addLog(msg) {
      let time = new Date().toLocaleTimeString();
      let div = document.createElement("div");
      div.innerText = `> [${time}] ${msg}`;
      logBox.appendChild(div);
      logBox.scrollTop = logBox.scrollHeight; // Tự cuộn xuống dưới
    }

    function speak(text) {
      if (!isSpeechOn || !('speechSynthesis' in window)) return;
      window.speechSynthesis.cancel(); // Dừng câu cũ
      let msg = new SpeechSynthesisUtterance();
      msg.text = text;
      msg.lang = 'vi-VN';
      msg.rate = 1.1;
      window.speechSynthesis.speak(msg);
    }

    // --- MAIN LOOP ---
    setInterval(function() {
      fetch("/status")
        .then(response => response.json())
        .then(data => {
          let char = data.pred;
          let conf = data.conf;

          // Update UI
          document.getElementById("liveChar").innerText = char;
          document.getElementById("conf").innerText = conf;
          document.getElementById("bar").style.width = conf + "%";

          // Màu sắc động
          let liveBox = document.getElementById("liveChar");
          if(char === "None") {
            liveBox.style.color = "#444";
            liveBox.style.textShadow = "none";
          } else {
            liveBox.style.color = "#fff";
            liveBox.style.textShadow = "0 0 20px #00f0ff";
          }

          // Logic ghép chữ
          if (lastState === "None" && char !== "None") {
            sentence += char;
            updateBoard();
            speak("Chữ " + char);
            addLog("AI Detected: " + char + " (" + conf + "%)");
          }
          lastState = char;
        })
        .catch(e => {
          // console.log(e); // Tắt log lỗi cho đỡ rác console
        });
    }, 500);
  </script>
</body>
</html>
)rawliteral";