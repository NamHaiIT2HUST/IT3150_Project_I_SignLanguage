import cv2
import json
import numpy as np
import tensorflow as tf
import requests
import time
import threading
from flask import Flask, Response

ESP32_IP = "192.168.1.73" 

MODEL_PATH = "model/model.tflite"
LABEL_PATH = "model/labels.json"

IMG_SIZE = 64
CONFIDENCE_THRESHOLD = 0.4 
SEND_INTERVAL = 0.3        

output_frame = None
lock = threading.Lock()
app = Flask(__name__)

# Load Model TFLite
print("🔄 Đang tải Model AI...")
try:
    with open(LABEL_PATH, "r") as f:
        label_map = json.load(f)
    LABELS = [label_map[str(i)] for i in range(len(label_map))]

    interpreter = tf.lite.Interpreter(model_path=MODEL_PATH)
    interpreter.allocate_tensors()
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    print("✅ Model đã tải thành công!")
except Exception as e:
    print(f"❌ Lỗi tải Model: {e}")
    exit()

def process_camera():
    global output_frame
    cap = cv2.VideoCapture(0)
    
    if not cap.isOpened():
        print("❌ Không thể mở Webcam!")
        return

    last_send_time = 0

    while True:
        success, frame = cap.read()
        if not success:
            continue

        frame = cv2.flip(frame, 1)

        display_frame = frame.copy()

        x1, y1, x2, y2 = 100, 100, 350, 350
        cv2.rectangle(display_frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
        roi = frame[y1:y2, x1:x2]

        try:
            img = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
            img = cv2.resize(img, (IMG_SIZE, IMG_SIZE))
            img = img / 255.0
            input_data = img.reshape(1, IMG_SIZE, IMG_SIZE, 1).astype(np.float32)

            interpreter.set_tensor(input_details[0]['index'], input_data)
            interpreter.invoke()
            output = interpreter.get_tensor(output_details[0]['index'])[0]

            class_id = int(np.argmax(output))
            confidence = float(output[class_id])
            label = LABELS[class_id]

            text = f"{label} ({confidence:.2f})"
            color = (0, 255, 0) if confidence > CONFIDENCE_THRESHOLD else (0, 0, 255)
            cv2.putText(display_frame, text, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.8, color, 2)

            now = time.time()
            if confidence > CONFIDENCE_THRESHOLD and (now - last_send_time > SEND_INTERVAL):
                try:
                    url = f"http://{ESP32_IP}/update?char={label}"
                    requests.get(url, timeout=0.1)
                    last_send_time = now
                except:
                    pass

        except Exception as e:
            print(f"AI Error: {e}")

        with lock:
            output_frame = display_frame.copy()

        time.sleep(0.01)

    cap.release()

def generate():
    global output_frame
    while True:
        with lock:
            if output_frame is None:
                continue
            (flag, encodedImage) = cv2.imencode(".jpg", output_frame)
            if not flag:
                continue

        yield(b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + 
              bytearray(encodedImage) + b'\r\n')

@app.route("/video")
def video_feed():
    return Response(generate(), mimetype = "multipart/x-mixed-replace; boundary=frame")

if __name__ == '__main__':
    t = threading.Thread(target=process_camera)
    t.daemon = True
    t.start()

    print("\n" + "="*60)
    print(f"🚀 HỆ THỐNG ĐÃ KHỞI ĐỘNG THÀNH CÔNG!")
    print(f"🎥 Video Stream (PC) : http://localhost:5000/video")
    print("-" * 60)
    print(f"👉 BẤM VÀO ĐÂY ĐỂ MỞ WEB: http://{ESP32_IP}")
    print("="*60 + "\n")

    app.run(host="0.0.0.0", port=5000, debug=False, use_reloader=False)