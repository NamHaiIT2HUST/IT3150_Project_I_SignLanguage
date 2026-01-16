import cv2
import json
import numpy as np
import tensorflow as tf
import requests
import time

MODEL_PATH = "model/model.tflite" #Đường dẫn đến "bộ não" đã train
LABEL_PATH = "model/labels.json"  #Đường dẫn đến "từ điển" (số 0 là A, 1 là B...)
IMG_SIZE = 64                     #Kích thước ảnh chuẩn (phải khớp với lúc train)

#Địa chỉ IP của ESP32
ESP32_IP = "192.168.1.73" 
SEND_INTERVAL = 0.5  

last_send_time = 0    
last_label = None     

#Danh sách label
with open(LABEL_PATH, "r") as f:
    label_map = json.load(f)
LABELS = [label_map[str(i)] for i in range(len(label_map))]
print("Labels:", LABELS)

#KHỞI TẠO TENSORFLOW LITE
#Load nhanh hơn đối với thiết bị biên
interpreter = tf.lite.Interpreter(model_path=MODEL_PATH)
interpreter.allocate_tensors()

#Lấy input, output của dữ liệu
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

cap = cv2.VideoCapture(0)
if not cap.isOpened():
    raise RuntimeError("❌ Cannot open webcam")

print("Press Q to quit")

#Xử lý real time
while True:
    ret, frame = cap.read()
    if not ret: break

    # Lật ngược ảnh như gương để thao tác tự nhiên hơn
    frame = cv2.flip(frame, 1)

    #ô để xác định khung tay
    x1, y1 = 100, 100
    x2, y2 = 350, 350
    roi = frame[y1:y2, x1:x2] 

    #Hiện chỗ đặt tay
    cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)

    #Tiền xử lý ảnh
    gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY) 
    gray = cv2.GaussianBlur(gray, (5, 5), 0)     
    gray = cv2.resize(gray, (IMG_SIZE, IMG_SIZE))
    gray = gray / 255.0 #Chuẩn hóa dữ liệu
    
    input_data = gray.reshape(1, IMG_SIZE, IMG_SIZE, 1).astype(np.float32)

    #Nạp dữ liệu thấy được vào model dể nhận diện
    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke()
    output = interpreter.get_tensor(output_details[0]['index'])[0] #

    #Tìm ra lớp có xác suất cao nhất
    class_id = int(np.argmax(output)) 
    confidence = float(output[class_id]) 
    label = LABELS[class_id] 

    #GỬI DỮ LIỆU SANG ESP32
    now = time.time()
    if confidence > 0.6:
        if (label != last_label) or (now - last_send_time > SEND_INTERVAL):
            try:
                requests.get(
                    f"http://{ESP32_IP}/update?char={label}",
                    timeout=0.2
                )
                last_send_time = now
                last_label = label
            except:
                pass 

    #Hiện kết quả
    text = f"{label} ({confidence:.2f})"
    cv2.putText(
        frame, text, (x1, y1 - 10),
        cv2.FONT_HERSHEY_SIMPLEX, 0.9,
        (0, 255, 0), 2
    )

    cv2.imshow("Sign Language Detection", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()