import cv2
import numpy as np
import tensorflow as tf
from flask import Flask, Response
import threading
import time
import requests  

ESP32_IP = "172.20.10.2"  

app = Flask(__name__)

try:
    model = tf.lite.Interpreter(model_path="model/model.tflite")
    model.allocate_tensors()
    input_details = model.get_input_details()
    output_details = model.get_output_details()
    import json
    with open('model/labels.json', 'r') as f:
        labels = json.load(f)
    print("✅ Đã tải Model thành công!")
except Exception as e:
    print("❌ Lỗi tải Model:", e)

current_char = "?"
frame_bytes = b''

def detect_loop():
    global current_char, frame_bytes
    cap = cv2.VideoCapture(0)
    
    while True:
        ret, frame = cap.read()
        if not ret: continue

        frame = cv2.flip(frame, 1)

        cv2.rectangle(frame, (100, 100), (400, 400), (0, 255, 0), 2)
        roi = frame[100:400, 100:400]

        confidence = 0.0
        text_display = "?" 

        if roi.size > 0:
            try:
                #Chuyển sang ảnh xám
                roi_gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
                
                #Resize về 64x64
                img = cv2.resize(roi_gray, (64, 64))
                
                #Reshape chuẩn (1, 64, 64, 1)
                img = img.reshape(1, 64, 64, 1).astype(np.float32) / 255.0
                
                #Dự đoán
                model.set_tensor(input_details[0]['index'], img)
                model.invoke()
                output = model.get_tensor(output_details[0]['index'])[0]
                
                idx = np.argmax(output) 
                confidence = output[idx]

                if str(idx) in labels:
                    raw_label = labels[str(idx)]
                    temp_char = raw_label.replace("-samples", "").replace("_samples", "").replace("samples", "").strip()
                else:
                    temp_char = "?"

                if confidence > 0.7:
                    current_char = temp_char
                    text_display = f"{current_char} ({confidence:.2f})"
                    
                    try:
                        url = f"http://{ESP32_IP}/update?char={current_char}"
                        requests.get(url, timeout=0.1)
                        print(f"📡 Đã gửi '{current_char}'")
                    except:
                        pass
                else:
                    text_display = f"? ({confidence:.2f})"

            except Exception as e:
                print(f"Lỗi: {e}") 

        #Hiển thị lên màn hình
        cv2.putText(frame, text_display, (100, 90), 
                   cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        ret, buffer = cv2.imencode('.jpg', frame)
        frame_bytes = buffer.tobytes()

threading.Thread(target=detect_loop, daemon=True).start()

@app.route('/video')
def video_feed():
    def generate():
        while True:
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')
            time.sleep(0.03)
    return Response(generate(), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)