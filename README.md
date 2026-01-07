# 🤟 Hệ thống Nhận diện Ngôn ngữ Ký hiệu (Sign Language Recognition System)

> **Đồ án I - Kỹ thuật Máy tính - Đại học Bách Khoa Hà Nội (HUST)**
>
> **Sinh viên:** Nguyễn Đào Nam Hải
> **MSSV:** 20235321
> **Lớp:** IT3150

![Python](https://img.shields.io/badge/Python-3.10+-blue.svg?style=flat&logo=python)
![TensorFlow](https://img.shields.io/badge/TensorFlow-2.x-orange.svg?style=flat&logo=tensorflow)
![ESP32](https://img.shields.io/badge/Hardware-ESP32-green.svg?style=flat&logo=arduino)
![Flask](https://img.shields.io/badge/Backend-Flask-lightgrey.svg?style=flat&logo=flask)

## 📖 Giới thiệu (Overview)

Dự án này xây dựng một hệ thống **AI + IoT** hỗ trợ giao tiếp cho người khiếm thính. Hệ thống sử dụng Camera máy tính để nhận diện 26 chữ cái ngôn ngữ ký hiệu Mỹ (ASL) theo thời gian thực và truyền kết quả hiển thị xuống vi điều khiển **ESP32** thông qua giao thức HTTP/Wi-Fi.

Mục tiêu là tạo ra một thiết bị hiển thị nhỏ gọn, giao diện thân thiện, giúp chuyển đổi cử chỉ tay thành văn bản ngay lập tức.

![Giao diện hệ thống](web_ui_detection.jpg)
*(Hình ảnh giao diện thực tế khi nhận diện chữ cái)*

---

## 🚀 Tính năng nổi bật (Key Features)

* **⚡ Xử lý thời gian thực:** Tốc độ 25-30 FPS nhờ kỹ thuật Đa luồng (Multi-threading).
* **🧠 Deep Learning:** Sử dụng mạng Nơ-ron Tích chập (CNN) được huấn luyện trên dataset ASL.
* **📡 IoT Streaming:** Truyền video mượt mà từ PC xuống ESP32 (MJPEG Streaming).
* **🎨 Giao diện Glassmorphism:** Thiết kế UI hiện đại, thân thiện trên màn hình web.
* **🔄 Đồng bộ dữ liệu:** ESP32 tự động cập nhật kết quả nhận diện liên tục.

---

## 📂 Cấu trúc thư mục (Project Structure)

Dựa trên cấu trúc mã nguồn thực tế:

```text
IT3150_PROJECT_I_SIGNLANGUAGE/
├── docs/                       # Tài liệu báo cáo (PDF)
├── esp32/
│   └── web/
│       ├── web.ino             # Code nạp cho mạch ESP32
│       └── web_interface.h     # Chứa mã HTML/CSS giao diện
├── pc/                         # --- MÃ NGUỒN CHÍNH TRÊN MÁY TÍNH ---
│   ├── dataset/                # Dữ liệu ảnh sau khi xử lý
│   ├── model/                  # Chứa model.tflite và labels.json
│   ├── raw_data/               # Dữ liệu thô (asl_alphabet_train)
│   ├── static/                 # Tài nguyên tĩnh (ảnh hướng dẫn, icon)
│   ├── prepare_dataset.py      # Code tiền xử lý dữ liệu
│   ├── train_model.py          # Code huấn luyện mô hình AI
│   ├── run_system.py           # Code SERVER chạy hệ thống
│   ├── realtime_detect.py      # Module nhận diện thời gian thực
│   ├── accuracy_chart.png      # Biểu đồ độ chính xác (tự sinh ra khi train)
│   └── loss_chart.png          # Biểu đồ hàm mất mát (tự sinh ra khi train)
├── requirements.txt            # Danh sách thư viện Python
└── README.md                   # Tài liệu hướng dẫn này
```
