import os
import cv2
import numpy as np
import tensorflow as tf
from sklearn.model_selection import train_test_split

DATASET_DIR = "dataset"
CLASSES = ["A", "B", "C"]
IMG_SIZE = 64
EPOCHS = 10
BATCH_SIZE = 16

X, y = [], []

for idx, cls in enumerate(CLASSES):
    cls_dir = os.path.join(DATASET_DIR, cls)
    for img_name in os.listdir(cls_dir):
        img_path = os.path.join(cls_dir, img_name)
        img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
        if img is None:
            continue
        img = img / 255.0
        X.append(img)
        y.append(idx)

X = np.array(X).reshape(-1, IMG_SIZE, IMG_SIZE, 1)
y = np.array(y)

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

model = tf.keras.Sequential([
    tf.keras.layers.Conv2D(8, (3,3), activation='relu', input_shape=(IMG_SIZE, IMG_SIZE, 1)),
    tf.keras.layers.MaxPooling2D(2,2),

    tf.keras.layers.Conv2D(16, (3,3), activation='relu'),
    tf.keras.layers.MaxPooling2D(2,2),

    tf.keras.layers.Flatten(),
    tf.keras.layers.Dense(32, activation='relu'),
    tf.keras.layers.Dense(len(CLASSES), activation='softmax')
])

model.compile(
    optimizer='adam',
    loss='sparse_categorical_crossentropy',
    metrics=['accuracy']
)

model.summary()

model.fit(
    X_train, y_train,
    validation_data=(X_test, y_test),
    epochs=EPOCHS,
    batch_size=BATCH_SIZE
)

os.makedirs("model", exist_ok=True)
model.save("model/sign_model.h5")

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

with open("model/sign_model.tflite", "wb") as f:
    f.write(tflite_model)

print("DONE: model/sign_model.tflite")
print("TFLite size:", len(tflite_model) / 1024, "KB")
