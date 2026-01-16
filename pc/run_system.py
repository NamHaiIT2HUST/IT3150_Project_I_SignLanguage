import cv2
import numpy as np
import tensorflow as tf
from flask import Flask, Response
import threading
import time
import requests  # Thư viện để gửi tin sang ESP32

# ==========================================
# CẤU HÌNH QUAN TRỌNG (SỬA IP CỦA ESP32 Ở ĐÂY)
# ==========================================
# Nhìn trên thanh địa chỉ trình duyệt: 172.20.10.2
ESP32_IP = "172.20.10.2"  
# ==========================================

app = Flask(__name__)

# Tải model (giữ nguyên đường dẫn của bạn)
try:
    model = tf.lite.Interpreter(model_path="model/model.tflite")
    model.allocate_tensors()
    input_details = model.get_input_details()
    output_details = model.get_output_details()
    # Load labels
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
        
        # Lật ảnh
        frame = cv2.flip(frame, 1)

        # Vẽ khung vuông
        cv2.rectangle(frame, (100, 100), (400, 400), (0, 255, 0), 2)
        roi = frame[100:400, 100:400]
        
        # Giá trị mặc định
        confidence = 0.0
        text_display = "?" 

        if roi.size > 0:
            try:
                # 1. Chuyển sang ảnh xám
                roi_gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
                
                # 2. Resize về 64x64
                img = cv2.resize(roi_gray, (64, 64))
                
                # 3. Reshape chuẩn (1, 64, 64, 1)
                img = img.reshape(1, 64, 64, 1).astype(np.float32) / 255.0
                
                # 4. Dự đoán
                model.set_tensor(input_details[0]['index'], img)
                model.invoke()
                output = model.get_tensor(output_details[0]['index'])[0]
                
                idx = np.argmax(output)      # Ra số (ví dụ: 12)
                confidence = output[idx]
                
                # ===> SỬA LỖI Ở ĐÂY: Thêm str() <===
                if str(idx) in labels:
                    raw_label = labels[str(idx)]
                    # Cắt bỏ mọi chữ "samples", "_samples", "-samples" cho sạch
                    temp_char = raw_label.replace("-samples", "").replace("_samples", "").replace("samples", "").strip()
                else:
                    temp_char = "?"

                # Logic gửi sang ESP32
                if confidence > 0.7: # Tăng độ tin cậy lên chút cho đỡ nhảy lung tung
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
                # In lỗi cụ thể hơn để dễ sửa
                print(f"Lỗi: {e}") 

        # Hiển thị lên màn hình
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