import cv2
import os
import random

# Cấu hình
RAW_DIR = "raw_data/asl_alphabet_train" 
OUT_DIR = "dataset"                    
IMG_SIZE = 64
MAX_IMAGES = 500 

os.makedirs(OUT_DIR, exist_ok=True)

classes_to_process = sorted(os.listdir(RAW_DIR)) 

print(f"🔄 Tìm thấy {len(classes_to_process)} lớp: {classes_to_process}")

for cls in classes_to_process:
    src_path = os.path.join(RAW_DIR, cls)

    if not os.path.isdir(src_path):
        continue

    dst_path = os.path.join(OUT_DIR, cls)
    os.makedirs(dst_path, exist_ok=True)

    images = os.listdir(src_path)

    random.shuffle(images)
    images = images[:MAX_IMAGES]

    count = 0
    for i, img_name in enumerate(images):
        try:
            img_path = os.path.join(src_path, img_name)
            img = cv2.imread(img_path)

            if img is None: continue

            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            resized = cv2.resize(gray, (IMG_SIZE, IMG_SIZE))

            cv2.imwrite(os.path.join(dst_path, f"{i}.jpg"), resized)
            count += 1
        except Exception as e:
            print(f"Lỗi ảnh {img_name}: {e}")

    print(f"✅ Đã xử lý {cls}: {count} ảnh")

print("🎉 DONE! Dataset đã sẵn sàng.")