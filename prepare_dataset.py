import cv2
import os
import random

RAW_DIR = "raw_data/asl_alphabet_train"
OUT_DIR = "dataset"
CLASSES = ["A-samples", "B-samples", "C-samples"]
IMG_SIZE = 64
MAX_IMAGES = 300  

os.makedirs(OUT_DIR, exist_ok=True)

for cls in CLASSES:
    src = os.path.join(RAW_DIR, cls)
    dst = os.path.join(OUT_DIR, cls)
    os.makedirs(dst, exist_ok=True)

    images = os.listdir(src)
    random.shuffle(images)
    images = images[:MAX_IMAGES]

    for i, img_name in enumerate(images):
        img_path = os.path.join(src, img_name)
        img = cv2.imread(img_path)

        if img is None:
            continue

        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        resized = cv2.resize(gray, (IMG_SIZE, IMG_SIZE))
        cv2.imwrite(os.path.join(dst, f"{i}.jpg"), resized)

    print(f"{cls}: {len(images)} images processed")

print("DONE!")
