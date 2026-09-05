"""PC server trung tâm (kiến trúc v2 - Edge AI).

Không còn chạy webcam/TensorFlow lúc runtime — camera + suy luận CNN đã chuyển
hẳn sang chạy on-device trên ESP32-S3-CAM (xem esp32/edge_inference/edge_inference.ino).
File này chỉ nhận kết quả nhận diện qua HTTP POST, lưu trạng thái/lịch sử, và host
dashboard web cho trình duyệt.

Chạy: python pc/server.py
"""
import threading
import time

from flask import Flask, jsonify, render_template, request

app = Flask(__name__)

HOLD_SECONDS = 0.8   # Ký hiệu phải giữ ổn định bấy nhiêu giây mới được ghi vào lịch sử
IDLE_TIMEOUT = 1.5   # Không nhận được dự đoán mới sau bấy nhiêu giây -> coi là "không có tay"

state_lock = threading.Lock()
state = {
    "char": None,
    "confidence": 0.0,
    "last_update": 0.0,
    "history": "",
    "pending_char": None,
    "pending_since": 0.0,
    "last_committed": None,
}


@app.route("/predict", methods=["POST"])
def predict():
    """Nhận kết quả suy luận từ ESP32-S3-CAM: {"char": "A", "confidence": 0.92}"""
    data = request.get_json(force=True, silent=True) or {}
    char = data.get("char")
    confidence = float(data.get("confidence", 0.0))
    if not char:
        return jsonify({"error": "missing 'char'"}), 400

    now = time.time()
    with state_lock:
        state["char"] = char
        state["confidence"] = confidence
        state["last_update"] = now

        if char != state["pending_char"]:
            state["pending_char"] = char
            state["pending_since"] = now
        elif (now - state["pending_since"] >= HOLD_SECONDS
              and char != state["last_committed"]):
            state["history"] += char
            state["last_committed"] = char

    return jsonify({"status": "ok"})


@app.route("/state")
def get_state():
    """Trình duyệt poll endpoint này để cập nhật dashboard theo thời gian thực."""
    with state_lock:
        is_idle = (time.time() - state["last_update"]) > IDLE_TIMEOUT
        return jsonify({
            "char": None if is_idle else state["char"],
            "confidence": 0.0 if is_idle else state["confidence"],
            "connected": not is_idle,
            "history": state["history"],
        })


@app.route("/clear", methods=["POST"])
def clear_history():
    with state_lock:
        state["history"] = ""
        state["last_committed"] = None
    return jsonify({"status": "ok"})


@app.route("/")
def index():
    return render_template("index.html")


if __name__ == "__main__":
    print("🚀 Dashboard chạy tại: http://<PC_IP>:5000  (ESP32-S3-CAM gửi kết quả tới /predict)")
    app.run(host="0.0.0.0", port=5000, debug=False)
