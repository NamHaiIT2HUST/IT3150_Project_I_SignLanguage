# ESP32-S3 Sign Language Recognition Project 🤟
**(Dự án Nhận diện Ký hiệu Tay sử dụng AI trên ESP32-S3)**

## 1. Giới thiệu (Overview)
Dự án này xây dựng một hệ thống nhúng có khả năng nhận diện cử chỉ tay (Sign Language) theo thời gian thực. Hệ thống sử dụng Camera của ESP32-S3 để thu thập hình ảnh, sau đó xử lý bằng mô hình Deep Learning (CNN) siêu nhẹ đã được huấn luyện và nhúng trực tiếp vào vi điều khiển.

* **Phiên bản:** Backup-version1
* **Tình trạng:** Hoàn thành phần mềm (Software Ready) - Đang chờ phần cứng (Hardware Pending).

## 2. Cấu trúc thư mục (Project Structure)
Dự án được chia thành các module rõ ràng:

├── Data_Collection/          # Module thu thập dữ liệu
│   ├── Python_Collector/       # Code Python dùng Webcam Laptop để chụp ảnh mẫu
│   └── Dataset_Raw/            # Dữ liệu thô (ảnh đã chụp: A, B, None)
│
├── ESP32_Firmware/           # Firmware nạp cho mạch ESP32-S3
│   └── ESP32_Inference/        # Code chính chạy AI
│       ├── ESP32_Inference.ino # Chương trình điều khiển trung tâm
│       ├── model_data.h        # "Bộ não" AI (đã train & convert sang C array)
│       ├── camera_pins.h       # Cấu hình chân cho ESP32-S3-CAM
│       └── webpage.h           # Giao diện Web hiển thị kết quả
│
└── README.md                   # Tài liệu hướng dẫn

## 3. Công nghệ sử dụng (Tech Stack)
* **Hardware:** ESP32-S3-CAM (AI Thinker / Freenove).
* **AI Training:** TensorFlow, Keras (Google Colab).
* **Embedded AI:** TensorFlow Lite for Microcontrollers (TFLite Micro).
* **Language:** Python (Training/Data), C++ (Arduino Firmware).

## 4. Kết quả Huấn luyện (Model Performance)
Mô hình đã được huấn luyện trên Google Colab với kết quả:
* **Architecture:** CNN (Convolutional Neural Network) - Tối ưu cho MCU.
* **Training Accuracy:** ~99.7%
* **Validation Accuracy:** 100%
* **Classes:** A, B, None.

## 5. Hướng dẫn chạy (How to Run)
### Bước 1: Nạp Code (Flash Firmware)
1.  Mở thư mục `3_ESP32_Firmware/ESP32_Inference` bằng Arduino IDE.
2.  Cài đặt thư viện: `TensorFlowLite_ESP32` (Version 1.0.0).
3.  Board config: `ESP32S3 Dev Module`, Partition Scheme: `OPI PSRAM`.
4.  Cập nhật Wifi SSID/Pass trong file `.ino`.
5.  Nạp code vào mạch.

### Bước 2: Kiểm thử (Testing)
1.  Mở Serial Monitor để lấy địa chỉ IP.
2.  Truy cập IP từ trình duyệt điện thoại/máy tính.
3.  Hệ thống sẽ hiển thị nhãn dự đoán (Prediction) và độ tin cậy (Confidence).

---
*Project by Nguyen_Dao_Nam_Hai (20235321)*