import os
import cv2
import json
import numpy as np
import tensorflow as tf
import matplotlib.pyplot as plt 
from sklearn.model_selection import train_test_split
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Conv2D, MaxPooling2D, Flatten, Dense, Dropout

DATASET_DIR = "dataset" #ảnh train data
MODEL_DIR = "model" #lưu sau khi train
IMG_SIZE = 48  # Giảm từ 64->48 để tensor arena vừa RAM khi chạy on-device trên ESP32
EPOCHS = 15 #số lần học dữ liệu
BATCH_SIZE = 32 #Số lần học trong 1 lần cập nhật

os.makedirs(MODEL_DIR, exist_ok=True)

#Lưu label
CLASSES = sorted([d for d in os.listdir(DATASET_DIR) if os.path.isdir(os.path.join(DATASET_DIR, d))])
print(f"🔥 Đang train cho {len(CLASSES)} lớp: {CLASSES}")

images = []
labels = []

print("🔄 Đang tải dữ liệu...")

#Giai đoạn tiền xử lý
for label_id, label_name in enumerate(CLASSES):
    folder = os.path.join(DATASET_DIR, label_name)
    for img_name in os.listdir(folder):
        img_path = os.path.join(folder, img_name)
        
        #Đọc ảnh đen trắng
        img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
        
        if img is not None:
            #Chỉnh ảnh về đúng kích thước cho model đọc
            img = cv2.resize(img, (IMG_SIZE, IMG_SIZE))
            images.append(img)
            labels.append(label_id)

#Chuẩn hóa cho model đọc nhanh hơn
images = np.array(images).reshape(-1, IMG_SIZE, IMG_SIZE, 1) / 255.0
labels = np.array(labels)

#Chia ra 80% để học, 20% để test -> K-fold
X_train, X_test, y_train, y_test = train_test_split(images, labels, test_size=0.2, random_state=42, stratify=labels)

#Cấu trúc mạng CNN gọn nhẹ để chạy được on-device trên ESP32 (TFLite Micro)
#padding="same" để activation map co theo đúng tỷ lệ pooling, tránh tensor
#trung gian phình to (bản cũ dùng padding mặc định "valid" khiến layer đầu
#tạo ra tensor 62x62x32, không vừa tensor arena của MCU)
model = Sequential([
    Conv2D(16, (3,3), activation="relu", padding="same", input_shape=(IMG_SIZE, IMG_SIZE, 1)),
    MaxPooling2D(2,2),

    Conv2D(32, (3,3), activation="relu", padding="same"),
    MaxPooling2D(2,2),

    Conv2D(32, (3,3), activation="relu", padding="same"),
    MaxPooling2D(2,2),

    Flatten(),

    Dense(64, activation="relu"),

    #Tránh overfitting
    Dropout(0.3),

    Dense(len(CLASSES), activation="softmax")
])

#Thiết lập để vẽ biểu đồ
model.compile(optimizer="adam", loss="sparse_categorical_crossentropy", metrics=["accuracy"])

print("🚀 Bắt đầu training...")
#Qúa trình huấn luyện
history = model.fit(X_train, y_train, validation_data=(X_test, y_test), epochs=EPOCHS, batch_size=BATCH_SIZE)

print("📊 Đang vẽ biểu đồ...")

#Vẽ biểu đồ Accuracy
plt.figure(figsize=(8, 5))
plt.plot(history.history['accuracy'], label='Train Accuracy', color='blue')
plt.plot(history.history['val_accuracy'], label='Validation Accuracy', color='orange')
plt.title('Độ chính xác mô hình (Accuracy)')
plt.xlabel('Epochs')
plt.ylabel('Accuracy')
plt.legend()
plt.grid(True)
plt.savefig('accuracy_chart.png') 
print("✅ Đã lưu accuracy_chart.png")

#Vẽ biểu đồ Loss
plt.figure(figsize=(8, 5))
plt.plot(history.history['loss'], label='Train Loss', color='blue')
plt.plot(history.history['val_loss'], label='Validation Loss', color='red')
plt.title('Hàm mất mát (Loss)')
plt.xlabel('Epochs')
plt.ylabel('Loss')
plt.legend()
plt.grid(True)
plt.savefig('loss_chart.png') 
print("✅ Đã lưu loss_chart.png")

#chuyển sang dạng TFLite Micro (int8) để chạy on-device trên ESP32
def representative_data_gen():
    for i in range(min(300, len(X_train))):
        yield [X_train[i:i+1].astype(np.float32)]

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_data_gen
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8
tflite_model = converter.convert()
with open(os.path.join(MODEL_DIR, "model.tflite"), "wb") as f:
    f.write(tflite_model)
print(f"✅ Đã lưu model.tflite (int8, {len(tflite_model)/1024:.1f} KB)")

#Lưu theo các nhãn (bỏ hậu tố "-samples"/"_samples" của tên thư mục dataset
#để tránh phải strip lại ở tầng suy luận/hiển thị)
def clean_label(name):
    return name.replace("-samples", "").replace("_samples", "").replace("samples", "").strip()

label_map = {i: clean_label(cls) for i, cls in enumerate(CLASSES)}
with open(os.path.join(MODEL_DIR, "labels.json"), "w") as f:
    json.dump(label_map, f, indent=2)

print("🎉 DONE! Đã lưu model và biểu đồ.")