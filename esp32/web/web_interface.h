const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Sign Language Assistant</title>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600;800&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg-color: #f4f6f9;       /* Nền sáng nhẹ */
      --card-bg: #ffffff;        /* Nền thẻ trắng */
      --primary: #2563eb;        /* Xanh dương hiện đại */
      --danger: #ef4444;         /* Đỏ */
      --success: #22c55e;        /* Xanh lá */
      --text-dark: #1e293b;
      --text-gray: #64748b;
    }

    body {
      background-color: var(--bg-color);
      color: var(--text-dark);
      font-family: 'Inter', sans-serif;
      margin: 0;
      padding: 20px;
      display: flex;
      justify-content: center;
      min-height: 100vh;
    }

    .container {
      display: grid;
      grid-template-columns: 1.5fr 1fr; /* Chia cột: Trái lớn, Phải nhỏ hơn */
      gap: 25px;
      width: 100%;
      max-width: 1200px;
    }

    /* --- CỘT TRÁI: CAMERA & BẢNG TRA CỨU --- */
    .left-column {
      display: flex;
      flex-direction: column;
      gap: 20px;
    }

    .video-card {
      background: var(--card-bg);
      padding: 15px;
      border-radius: 16px;
      box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
      text-align: center;
    }

    .section-title {
      font-size: 1.1rem;
      font-weight: 600;
      color: var(--text-gray);
      margin-bottom: 10px;
      text-align: left;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }

    img#video {
      width: 100%;
      border-radius: 12px;
      border: 2px solid #e2e8f0;
    }

    .cheat-sheet-card {
      background: var(--card-bg);
      padding: 20px;
      border-radius: 16px;
      box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
    }
    
    /* Ảnh bảng chữ cái mẫu (Link online) */
    .cheat-sheet-img {
      width: 100%;
      border-radius: 8px;
    }

    /* --- CỘT PHẢI: KẾT QUẢ & ĐIỀU KHIỂN --- */
    .right-column {
      display: flex;
      flex-direction: column;
      gap: 20px;
    }

    .control-card {
      background: var(--card-bg);
      padding: 30px;
      border-radius: 16px;
      box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.1);
      display: flex;
      flex-direction: column;
      align-items: center;
      text-align: center;
      height: 100%;
    }

    /* Đồng hồ đếm ngược */
    .timer-ring {
      position: relative;
      width: 120px;
      height: 120px;
      margin-bottom: 20px;
    }
    
    .timer-svg { transform: rotate(-90deg); }
    .timer-circle-bg { fill: none; stroke: #e2e8f0; stroke-width: 8; }
    .timer-circle-fg { fill: none; stroke: var(--primary); stroke-width: 8; stroke-linecap: round; transition: stroke-dashoffset 1s linear; }
    .timer-text {
      position: absolute; top: 50%; left: 50%;
      transform: translate(-50%, -50%);
      font-size: 2rem; font-weight: 800; color: var(--text-dark);
    }

    /* Kết quả nhận diện */
    .result-box {
      margin-bottom: 30px;
    }
    .big-letter {
      font-size: 6rem;
      font-weight: 800;
      color: var(--primary);
      line-height: 1;
    }
    .label-desc { font-size: 0.9rem; color: var(--text-gray); margin-top: 5px; }

    /* Lịch sử */
    .history-area {
      width: 100%;
      background: #f8fafc;
      border: 1px solid #e2e8f0;
      border-radius: 10px;
      padding: 15px;
      min-height: 80px;
      margin-bottom: 20px;
      text-align: left;
      font-family: monospace;
      font-size: 1.2rem;
      color: #334155;
      word-wrap: break-word;
    }

    /* Nút bấm */
    .btn-group { display: flex; gap: 10px; width: 100%; }
    
    button {
      flex: 1;
      padding: 15px;
      border: none;
      border-radius: 10px;
      font-size: 1rem;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.2s;
    }

    .btn-start { background-color: var(--primary); color: white; }
    .btn-start:hover { background-color: #1d4ed8; }
    .btn-start.active { background-color: var(--danger); } /* Đổi màu khi đang chạy */

    .btn-clear { background-color: #e2e8f0; color: var(--text-dark); }
    .btn-clear:hover { background-color: #cbd5e1; }

    /* Responsive mobile */
    @media (max-width: 768px) {
      .container { grid-template-columns: 1fr; }
      .left-column { order: 2; } /* Đẩy camera xuống dưới trên mobile */
      .right-column { order: 1; }
    }
  </style>
</head>
<body>

  <div class="container">
    
    <div class="left-column">
      <div class="video-card">
        <div class="section-title">📷 Camera Live Feed</div>
        <img id="video" src="http://192.168.1.70:5000/video" alt="Video Feed">
      </div>

      <div class="cheat-sheet-card">
        <div class="section-title">📖 Bảng Ký Hiệu Mẫu (A-Z)</div>
        <img src="http://192.168.1.70:5000/static/asl_table_left_hand.jpg" class="cheat-sheet-img" alt="Bảng ký hiệu">
      </div>
    </div>

    <div class="right-column">
      <div class="control-card">
        
        <div class="timer-ring">
          <svg class="timer-svg" width="120" height="120">
            <circle class="timer-circle-bg" cx="60" cy="60" r="50"></circle>
            <circle class="timer-circle-fg" cx="60" cy="60" r="50" stroke-dasharray="314" stroke-dashoffset="0"></circle>
          </svg>
          <div class="timer-text" id="timer-display">5</div>
        </div>

        <div class="result-box">
          <div class="label-desc">Ký tự nhận diện</div>
          <div class="big-letter" id="live-char">--</div>
        </div>

        <div class="section-title" style="width:100%">📝 Văn bản đã tạo:</div>
        <div class="history-area" id="history-text"></div>

        <div class="btn-group">
          <button class="btn-start" id="btn-toggle" onclick="toggleSystem()">▶ BẮT ĐẦU</button>
          <button class="btn-clear" onclick="clearHistory()">🗑 Xóa</button>
        </div>

      </div>
    </div>

  </div>

  <script>
    let timeLeft = 5;
    let maxTime = 5;
    let isRunning = false; // Trạng thái: false = dừng, true = chạy
    let currentDetect = "";
    let historyStr = "";

    // Cập nhật vòng tròn Timer
    const circle = document.querySelector('.timer-circle-fg');
    const radius = circle.r.baseVal.value;
    const circumference = radius * 2 * Math.PI;
    circle.style.strokeDasharray = `${circumference} ${circumference}`;

    function setProgress(percent) {
      const offset = circumference - (percent / 100) * circumference;
      circle.style.strokeDashoffset = offset;
    }

    // 1. Logic Nút Bấm START / STOP
    function toggleSystem() {
      const btn = document.getElementById("btn-toggle");
      isRunning = !isRunning;

      if (isRunning) {
        // Chuyển sang trạng thái chạy
        btn.innerText = "⏹ DỪNG LẠI";
        btn.classList.add("active");
        timeLeft = 5; // Reset timer
        setProgress(100);
      } else {
        // Chuyển sang trạng thái dừng
        btn.innerText = "▶ BẮT ĐẦU";
        btn.classList.remove("active");
        // Reset hiển thị về mặc định
        document.getElementById("timer-display").innerText = "5";
        setProgress(0); 
      }
    }

    // 2. Nhận dữ liệu từ ESP32 (Liên tục chạy ngầm để cập nhật Live Char)
    setInterval(() => {
      fetch('/get')
        .then(res => res.text())
        .then(data => {
          currentDetect = data;
          // Chỉ hiện chữ cái to lên màn hình, chưa chốt vào lịch sử
          if(data === "?" || data === "") {
             document.getElementById("live-char").innerText = "--";
             document.getElementById("live-char").style.color = "#ccc";
          } else {
             document.getElementById("live-char").innerText = data;
             document.getElementById("live-char").style.color = "#2563eb";
          }
        })
        .catch(e => console.log("Waiting connection..."));
    }, 300);

    // 3. Logic Đếm Ngược (Chỉ chạy khi isRunning = true)
    setInterval(() => {
      if (!isRunning) return; // Nếu chưa bấm Start thì không làm gì cả

      timeLeft--;
      document.getElementById("timer-display").innerText = timeLeft;
      
      // Cập nhật vòng tròn
      let percent = (timeLeft / maxTime) * 100;
      setProgress(percent);

      if (timeLeft <= 0) {
        // HẾT GIỜ: Chốt chữ cái
        if (currentDetect !== "?" && currentDetect !== "" && currentDetect !== "--") {
          historyStr += currentDetect;
          document.getElementById("history-text").innerText = historyStr;
        }
        
        // Reset vòng lặp mới
        timeLeft = 5;
        setProgress(100);
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