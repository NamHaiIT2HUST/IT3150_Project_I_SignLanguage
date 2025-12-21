import cv2
import os
import time

# --- CẤU HÌNH ---
DATA_DIR = "../Dataset_Raw" 
IMG_SIZE = 96               

# DANH SÁCH NHÃN ĐẦY ĐỦ (A-Z, 0-9, Đặc biệt)
LABELS = [
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'Space', 'Delete', 'None'
]

# Tạo thư mục
if not os.path.exists(DATA_DIR):
    os.makedirs(DATA_DIR)
for label in LABELS:
    path = os.path.join(DATA_DIR, label)
    if not os.path.exists(path):
        os.makedirs(path)

# Mở Webcam
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Không mở được Webcam!")
    exit()

current_label = 'A' 
count = 0           

print("---------------------------------------------------------")
print("--- HƯỚNG DẪN SỬ DỤNG (UPDATE A-Z, 0-9) ---")
print("1. Phím A-Z: Chụp chữ cái.")
print("2. Phím 0-9: Chụp các con số.")
print("3. Phím '[' (cạnh chữ P) -> Chọn 'Space'")
print("4. Phím ']' (cạnh phím [) -> Chọn 'Delete'")
print("5. Phím '-' (dấu trừ)    -> Chọn 'None'")
print("6. GIỮ PHÍM KHOẢNG TRẮNG (SPACEBAR) ĐỂ CHỤP.")
print("7. Bấm ESC để thoát.")
print("---------------------------------------------------------")

while True:
    ret, frame = cap.read()
    if not ret: break

    frame = cv2.flip(frame, 1) # Lật gương
    
    # Vẽ khung chụp
    h, w, c = frame.shape
    start_x, start_y = int(w/2 - 150), int(h/2 - 150)
    end_x, end_y = int(w/2 + 150), int(h/2 + 150)
    
    roi = frame[start_y:end_y, start_x:end_x]
    
    if roi.size != 0:
        roi_gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
        roi_resized = cv2.resize(roi_gray, (IMG_SIZE, IMG_SIZE))

    # --- LOGIC CHỤP ẢNH ---
    keys = cv2.waitKey(1) & 0xFF
    
    # Phím Spacebar (Mã 32) để chụp
    if keys == 32: 
        timestamp = int(time.time() * 1000)
        img_name = f"{DATA_DIR}/{current_label}/{timestamp}.jpg"
        cv2.imwrite(img_name, roi_resized)
        count += 1
        print(f"Đã lưu: {current_label} ({count})")
        cv2.rectangle(frame, (start_x, start_y), (end_x, end_y), (0, 255, 0), 3)
    else:
        cv2.rectangle(frame, (start_x, start_y), (end_x, end_y), (0, 0, 255), 2)

    # Hiển thị thông tin
    cv2.putText(frame, f"Label: {current_label}", (10, 40), cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0, 255, 255), 2)
    cv2.putText(frame, f"Count: {count}", (10, 80), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)
    cv2.putText(frame, "Hold SPACE to Capture", (10, 450), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
    
    cv2.imshow("Data Collector", frame)
    if roi.size != 0:
        cv2.imshow("ROI (96x96)", roi_resized)

    # --- XỬ LÝ PHÍM CHỨC NĂNG ---
    
    # 1. Thoát (ESC - Mã 27)
    if keys == 27: 
        print("Đang thoát...")
        break
    
    # 2. Chọn chữ cái (a-z)
    if keys >= ord('a') and keys <= ord('z'):
        current_label = chr(keys).upper()
        # Đếm lại ảnh
        path = os.path.join(DATA_DIR, current_label)
        count = len(os.listdir(path)) if os.path.exists(path) else 0

    # 3. Chọn số (0-9)
    if keys >= ord('0') and keys <= ord('9'):
        current_label = chr(keys)
        path = os.path.join(DATA_DIR, current_label)
        count = len(os.listdir(path)) if os.path.exists(path) else 0

    # 4. Các nút đặc biệt mới
    # Phím '[' (Mã 91) -> Space
    if keys == 91:   
        current_label = 'Space'
        path = os.path.join(DATA_DIR, current_label)
        count = len(os.listdir(path)) if os.path.exists(path) else 0
        
    # Phím ']' (Mã 93) -> Delete
    if keys == 93: 
        current_label = 'Delete'
        path = os.path.join(DATA_DIR, current_label)
        count = len(os.listdir(path)) if os.path.exists(path) else 0

    # Phím '-' (Mã 45) -> None
    if keys == 45: 
        current_label = 'None'
        path = os.path.join(DATA_DIR, current_label)
        count = len(os.listdir(path)) if os.path.exists(path) else 0

cap.release()
cv2.destroyAllWindows()