import os
import cv2
import json
import numpy as np
import tensorflow as tf
from sklearn.model_selection import train_test_split
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Conv2D, MaxPooling2D, Flatten, Dense, Dropout

DATASET_DIR = "dataset"
MODEL_DIR = "model"
IMG_SIZE = 64
EPOCHS = 10
BATCH_SIZE = 32

CLASSES = [
    "A-samples",
    "B-samples",
    "C-samples"
]

os.makedirs(MODEL_DIR, exist_ok=True)

images = []
labels = []

print("Loading dataset...")

for label_id, label_name in enumerate(CLASSES):
    folder = os.path.join(DATASET_DIR, label_name)

    if not os.path.exists(folder):
        raise Exception(f"❌ Folder not found: {folder}")

    for img_name in os.listdir(folder):
        img_path = os.path.join(folder, img_name)

        img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
        if img is None:
            continue

        img = cv2.resize(img, (IMG_SIZE, IMG_SIZE))
        img = img / 255.0

        images.append(img)
        labels.append(label_id)

print(f"Total images: {len(images)}")

X = np.array(images).reshape(-1, IMG_SIZE, IMG_SIZE, 1)
y = np.array(labels)

# =========================
# TRAIN / TEST SPLIT
# =========================
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42, stratify=y
)

# =========================
# BUILD MODEL
# =========================
model = Sequential([
    Conv2D(16, (3,3), activation="relu", input_shape=(IMG_SIZE, IMG_SIZE, 1)),
    MaxPooling2D(2,2),

    Conv2D(32, (3,3), activation="relu"),
    MaxPooling2D(2,2),

    Flatten(),
    Dense(64, activation="relu"),
    Dropout(0.3),
    Dense(len(CLASSES), activation="softmax")
])

model.compile(
    optimizer="adam",
    loss="sparse_categorical_crossentropy",
    metrics=["accuracy"]
)

model.summary()

# =========================
# TRAIN
# =========================
model.fit(
    X_train, y_train,
    validation_data=(X_test, y_test),
    epochs=EPOCHS,
    batch_size=BATCH_SIZE
)

# =========================
# SAVE TFLITE MODEL
# =========================
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

tflite_path = os.path.join(MODEL_DIR, "model.tflite")
with open(tflite_path, "wb") as f:
    f.write(tflite_model)

print(f"✅ Saved model to {tflite_path}")

# =========================
# SAVE LABEL MAP (CỰC KỲ QUAN TRỌNG)
# =========================
label_map = {i: cls.replace("-samples", "") for i, cls in enumerate(CLASSES)}

labels_path = os.path.join(MODEL_DIR, "labels.json")
with open(labels_path, "w") as f:
    json.dump(label_map, f, indent=2)

print("✅ Saved labels:", label_map)
