import cv2
import os
import random

# Cấu hình
RAW_DIR = "raw_data/asl_alphabet_train" # Thư mục gốc chứa ảnh tải về
OUT_DIR = "dataset"                     # Thư mục chứa ảnh đã resize (64x64)
IMG_SIZE = 64
MAX_IMAGES = 500  # Lấy 500 ảnh mỗi chữ cái (để train cho nhanh, máy mạnh thì tăng lên 1000)

os.makedirs(OUT_DIR, exist_ok=True)

# 1. Tự động lấy danh sách tất cả các thư mục trong raw_data (A, B, C... Z, del, space...)
# Hoặc nếu muốn chỉ định A-Z thôi thì dùng dòng dưới:
# import string
# classes_to_process = list(string.ascii_uppercase) # ['A', 'B', ..., 'Z']

classes_to_process = sorted(os.listdir(RAW_DIR)) # Lấy tất cả thư mục có trong raw_data

print(f"🔄 Tìm thấy {len(classes_to_process)} lớp: {classes_to_process}")

for cls in classes_to_process:
    src_path = os.path.join(RAW_DIR, cls)
    
    # Bỏ qua nếu không phải là thư mục
    if not os.path.isdir(src_path):
        continue

    # Tạo thư mục đích (Ví dụ: dataset/A)
    dst_path = os.path.join(OUT_DIR, cls)
    os.makedirs(dst_path, exist_ok=True)

    images = os.listdir(src_path)
    
    # Shuffle để lấy ngẫu nhiên, tránh lấy toàn ảnh giống nhau
    random.shuffle(images)
    images = images[:MAX_IMAGES]

    count = 0
    for i, img_name in enumerate(images):
        try:
            img_path = os.path.join(src_path, img_name)
            img = cv2.imread(img_path)

            if img is None: continue

            # Xử lý: Grayscale -> Resize
            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            resized = cv2.resize(gray, (IMG_SIZE, IMG_SIZE))
            
            # Lưu ảnh
            cv2.imwrite(os.path.join(dst_path, f"{i}.jpg"), resized)
            count += 1
        except Exception as e:
            print(f"Lỗi ảnh {img_name}: {e}")

    print(f"✅ Đã xử lý {cls}: {count} ảnh")

print("🎉 DONE! Dataset đã sẵn sàng.")