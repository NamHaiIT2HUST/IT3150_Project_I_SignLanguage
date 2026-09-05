import os
import cv2
import json
import numpy as np
import tensorflow as tf
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.model_selection import train_test_split
from sklearn.metrics import confusion_matrix, classification_report
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

#Bỏ hậu tố "-samples"/"_samples" của tên thư mục dataset để ra tên chữ cái sạch
#(dùng chung cho label_map luu ra va label hiển thị trên confusion matrix)
def clean_label(name):
    return name.replace("-samples", "").replace("_samples", "").replace("samples", "").strip()

DISPLAY_LABELS = [clean_label(c) for c in CLASSES]

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

#Data augmentation: chỉ áp dụng cho tập train (giữ nguyên X_test để đánh giá công bằng).
#Dataset hiện chỉ ~80 ảnh train/lớp - augment giúp mô hình thấy nhiều biến thể góc
#xoay/độ sáng hơn, dù không giải quyết được gốc rễ việc vài chữ (M/N/S/T/E, K/V/W,
#R/U) có hình tay giống nhau thật sự (xem confusion_matrix.png).
AUGMENT_MULTIPLIER = 3  # Mỗi ảnh gốc sinh thêm bấy nhiêu bản augment

def augment_image(img_2d):
    img_u8 = (img_2d * 255).astype(np.uint8)

    angle = np.random.uniform(-15, 15)
    rot_m = cv2.getRotationMatrix2D((IMG_SIZE // 2, IMG_SIZE // 2), angle, 1.0)
    img_u8 = cv2.warpAffine(img_u8, rot_m, (IMG_SIZE, IMG_SIZE), borderMode=cv2.BORDER_REPLICATE)

    tx, ty = np.random.randint(-4, 5, size=2)
    shift_m = np.float32([[1, 0, tx], [0, 1, ty]])
    img_u8 = cv2.warpAffine(img_u8, shift_m, (IMG_SIZE, IMG_SIZE), borderMode=cv2.BORDER_REPLICATE)

    alpha = np.random.uniform(0.8, 1.2)  # contrast
    beta = np.random.uniform(-20, 20)    # brightness
    img_u8 = np.clip(img_u8.astype(np.float32) * alpha + beta, 0, 255).astype(np.uint8)

    return img_u8 / 255.0

print(f"🔄 Đang augment dữ liệu train (x{AUGMENT_MULTIPLIER})...")
aug_images = [augment_image(img[:, :, 0]) for img in X_train for _ in range(AUGMENT_MULTIPLIER)]
aug_labels = [label for label in y_train for _ in range(AUGMENT_MULTIPLIER)]

X_train = np.concatenate([X_train, np.array(aug_images).reshape(-1, IMG_SIZE, IMG_SIZE, 1)])
y_train = np.concatenate([y_train, np.array(aug_labels)])
print(f"✅ Tập train sau augment: {len(X_train)} ảnh")

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
plt.savefig('static/accuracy_chart.png')
print("✅ Đã lưu static/accuracy_chart.png")

#Vẽ biểu đồ Loss
plt.figure(figsize=(8, 5))
plt.plot(history.history['loss'], label='Train Loss', color='blue')
plt.plot(history.history['val_loss'], label='Validation Loss', color='red')
plt.title('Hàm mất mát (Loss)')
plt.xlabel('Epochs')
plt.ylabel('Loss')
plt.legend()
plt.grid(True)
plt.savefig('static/loss_chart.png')
print("✅ Đã lưu static/loss_chart.png")

#Confusion matrix trên tập test: biết chữ nào hay bị nhầm với chữ nào, để
#quyết định nên thu thêm dữ liệu tràn lan hay nhắm đúng vài cặp hay nhầm
print("🔍 Đang tính confusion matrix...")
y_pred_classes = np.argmax(model.predict(X_test, verbose=0), axis=1)

cm = confusion_matrix(y_test, y_pred_classes)
plt.figure(figsize=(12, 10))
sns.heatmap(cm, annot=True, fmt="d", cmap="Blues",
            xticklabels=DISPLAY_LABELS, yticklabels=DISPLAY_LABELS)
plt.title("Confusion Matrix")
plt.xlabel("Dự đoán")
plt.ylabel("Thực tế")
plt.tight_layout()
plt.savefig("static/confusion_matrix.png")
print("✅ Đã lưu static/confusion_matrix.png")

report = classification_report(y_test, y_pred_classes, target_names=DISPLAY_LABELS)
print(report)
with open(os.path.join(MODEL_DIR, "classification_report.txt"), "w", encoding="utf-8") as f:
    f.write(report)
print("✅ Đã lưu classification_report.txt")

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

#Lưu theo các nhãn (đã bỏ hậu tố "-samples"/"_samples" ở DISPLAY_LABELS phía trên
#để tránh phải strip lại ở tầng suy luận/hiển thị)
label_map = {i: label for i, label in enumerate(DISPLAY_LABELS)}
with open(os.path.join(MODEL_DIR, "labels.json"), "w") as f:
    json.dump(label_map, f, indent=2)

print("🎉 DONE! Đã lưu model và biểu đồ.")