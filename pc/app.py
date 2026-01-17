from flask import Flask, render_template, Response
import cv2
import numpy as np
import tensorflow as tf
import json
import time

app = Flask(__name__)

# --- CONFIG ---
MODEL_PATH = "model/model.tflite"
LABEL_PATH = "model/labels.json"
IMG_SIZE = 64

# --- LOAD MODEL ---
interpreter = tf.lite.Interpreter(model_path=MODEL_PATH)
interpreter.allocate_tensors()
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

with open(LABEL_PATH, "r") as f:
    label_map = json.load(f)
LABELS = [label_map[str(i)] for i in range(len(label_map))]

# Biến lưu kết quả hiện tại
current_prediction = "?"

def generate_frames():
    global current_prediction
    cap = cv2.VideoCapture(0)
    
    while True:
        success, frame = cap.read()
        if not success:
            break
            
        frame = cv2.flip(frame, 1)

        # 1. Cắt vùng ROI
        x1, y1, x2, y2 = 100, 100, 350, 350
        roi = frame[y1:y2, x1:x2]
        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)

        # 2. Nhận diện AI
        try:
            gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
            gray = cv2.GaussianBlur(gray, (5, 5), 0)
            gray = cv2.resize(gray, (IMG_SIZE, IMG_SIZE))
            gray = gray / 255.0
            input_data = gray.reshape(1, IMG_SIZE, IMG_SIZE, 1).astype(np.float32)

            interpreter.set_tensor(input_details[0]['index'], input_data)
            interpreter.invoke()
            output = interpreter.get_tensor(output_details[0]['index'])[0]
            
            class_id = int(np.argmax(output))
            confidence = float(output[class_id])
            
            if confidence > 0.6:
                current_prediction = LABELS[class_id] # Cập nhật biến toàn cục
                cv2.putText(frame, f"{current_prediction} ({confidence:.2f})", 
                           (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 255, 0), 2)
            else:
                current_prediction = "?"
                
        except Exception as e:
            pass

        # 3. Stream ảnh
        ret, buffer = cv2.imencode('.jpg', frame)
        frame = buffer.tobytes()
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
    
    cap.release()

# --- ROUTES ---

@app.route('/')
def index():
    # Render file HTML trong folder templates
    return render_template('index.html')

@app.route('/video_feed')
def video_feed():
    # Stream video
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/get_result')
def get_result():
    # API để Javascript lấy chữ cái về
    return current_prediction

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=True)