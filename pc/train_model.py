import os
import cv2
import json
import numpy as np
import tensorflow as tf
import matplotlib.pyplot as plt  # <--- Thêm thư viện này
from sklearn.model_selection import train_test_split
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Conv2D, MaxPooling2D, Flatten, Dense, Dropout

DATASET_DIR = "dataset"
MODEL_DIR = "model"
IMG_SIZE = 64
EPOCHS = 15          
BATCH_SIZE = 32

os.makedirs(MODEL_DIR, exist_ok=True)

# 1. Tự động quét danh sách lớp
CLASSES = sorted([d for d in os.listdir(DATASET_DIR) if os.path.isdir(os.path.join(DATASET_DIR, d))])
print(f"🔥 Đang train cho {len(CLASSES)} lớp: {CLASSES}")

images = []
labels = []

print("🔄 Đang tải dữ liệu...")
for label_id, label_name in enumerate(CLASSES):
    folder = os.path.join(DATASET_DIR, label_name)
    for img_name in os.listdir(folder):
        img_path = os.path.join(folder, img_name)
        img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
        if img is not None:
            img = cv2.resize(img, (IMG_SIZE, IMG_SIZE))
            images.append(img)
            labels.append(label_id)

images = np.array(images).reshape(-1, IMG_SIZE, IMG_SIZE, 1) / 255.0
labels = np.array(labels)

# Split Data
X_train, X_test, y_train, y_test = train_test_split(images, labels, test_size=0.2, random_state=42, stratify=labels)

# 2. Build Model
model = Sequential([
    Conv2D(32, (3,3), activation="relu", input_shape=(IMG_SIZE, IMG_SIZE, 1)),
    MaxPooling2D(2,2),
    Conv2D(64, (3,3), activation="relu"),
    MaxPooling2D(2,2),
    Conv2D(64, (3,3), activation="relu"),
    MaxPooling2D(2,2),
    Flatten(),
    Dense(128, activation="relu"),
    Dropout(0.4),
    Dense(len(CLASSES), activation="softmax")
])

model.compile(optimizer="adam", loss="sparse_categorical_crossentropy", metrics=["accuracy"])

# 3. Train Model (Lưu kết quả vào biến 'history')
print("🚀 Bắt đầu training...")
history = model.fit(X_train, y_train, validation_data=(X_test, y_test), epochs=EPOCHS, batch_size=BATCH_SIZE)

# ==========================================
# 👇 PHẦN MỚI: VẼ VÀ LƯU BIỂU ĐỒ 👇
# ==========================================
print("📊 Đang vẽ biểu đồ...")

# 1. Vẽ biểu đồ Accuracy
plt.figure(figsize=(8, 5))
plt.plot(history.history['accuracy'], label='Train Accuracy', color='blue')
plt.plot(history.history['val_accuracy'], label='Validation Accuracy', color='orange')
plt.title('Độ chính xác mô hình (Accuracy)')
plt.xlabel('Epochs')
plt.ylabel('Accuracy')
plt.legend()
plt.grid(True)
plt.savefig('accuracy_chart.png') # Lưu thành file ảnh
print("✅ Đã lưu accuracy_chart.png")

# 2. Vẽ biểu đồ Loss
plt.figure(figsize=(8, 5))
plt.plot(history.history['loss'], label='Train Loss', color='blue')
plt.plot(history.history['val_loss'], label='Validation Loss', color='red')
plt.title('Hàm mất mát (Loss)')
plt.xlabel('Epochs')
plt.ylabel('Loss')
plt.legend()
plt.grid(True)
plt.savefig('loss_chart.png') # Lưu thành file ảnh
print("✅ Đã lưu loss_chart.png")

# Save Model
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()
with open(os.path.join(MODEL_DIR, "model.tflite"), "wb") as f:
    f.write(tflite_model)

label_map = {i: cls for i, cls in enumerate(CLASSES)}
with open(os.path.join(MODEL_DIR, "labels.json"), "w") as f:
    json.dump(label_map, f, indent=2)

print("🎉 DONE! Đã lưu model và biểu đồ.")