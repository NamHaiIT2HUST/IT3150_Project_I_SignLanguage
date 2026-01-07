# 🤟 Hệ thống Nhận diện Ngôn ngữ Ký hiệu (Sign Language Recognition System)

> **IT3150_PROJECT I
>
> **Tác giả:** Nguyễn Đào Nam Hải
> **Lớp:** 755566

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

* **⚡ Xử lý thời gian thực:** Tốc độ 25-30 FPS nhờ kỹ thuật Đa luồng (Multi-threading) trên Python.
* **🧠 Deep Learning:** Sử dụng mạng Nơ-ron Tích chập (CNN) với độ chính xác >90%.
* **📡 IoT Streaming:** Truyền video mượt mà từ PC xuống ESP32 (MJPEG Streaming) với độ trễ thấp (<300ms).
* **🎨 Giao diện Glassmorphism:** Thiết kế UI hiện đại, hiệu ứng kính mờ, hiển thị tốt trên trình duyệt và màn hình di động.
* **🔄 Đồng bộ dữ liệu:** ESP32 tự động cập nhật kết quả nhận diện mỗi 300ms mà không cần tải lại trang (AJAX/Fetch API).

---

## 🛠️ Công nghệ sử dụng (Tech Stack)

### Phần cứng
* **Máy tính (PC/Laptop):** Chạy Server xử lý ảnh và AI.
* **ESP32 (NodeMCU/DevKit):** Đóng vai trò Web Client hiển thị.
* **Webcam:** Thu nhận hình ảnh đầu vào.

### Phần mềm
* **Ngôn ngữ:** Python 3.10+, C++ (Arduino).
* **AI Framework:** TensorFlow, Keras.
* **Xử lý ảnh:** OpenCV, NumPy.
* **Backend Server:** Flask, Threading.
* **Frontend:** HTML5, CSS3, JavaScript.

---

## 📂 Cấu trúc thư mục (Project Structure)

```text
IT3150_Project_I_SignLanguage/
├── pc/                         # --- PHẦN MÃ NGUỒN MÁY TÍNH ---
│   ├── dataset/                # Dữ liệu ảnh sau khi tiền xử lý
│   ├── model/                  # Chứa file model.tflite và labels.json
│   ├── static/                 # CSS, JS, Images cho giao diện Web
│   ├── templates/              # File HTML giao diện
│   ├── prepare_dataset.py      # Code tiền xử lý dữ liệu (Cắt, resize)
│   ├── train_model.py          # Code huấn luyện mô hình AI
│   ├── create_matrix.py        # Code tạo Confusion Matrix
│   └── run_system.py           # Code CHÍNH chạy hệ thống (Server)
│
├── esp32/                      # --- PHẦN MÃ NGUỒN NHÚNG ---
│   └── web/
│       └── web.ino             # Code nạp cho mạch ESP32
│
├── raw_data/                   # Thư mục chứa dataset gốc (ASL Alphabet)
├── requirements.txt            # Danh sách thư viện cần cài đặt
└── README.md                   # Tài liệu hướng dẫn này
```

## ⚙️ Hướng dẫn cài đặt (Installation)
### 1. Thiết lập môi trường trên PC
Cài đặt các thư viện Python cần thiết:

pip install tensorflow opencv-python flask numpy matplotlib seaborn scikit-learn
2. Chuẩn bị dữ liệu và Huấn luyện
Nếu bạn chưa có file model, hãy thực hiện lần lượt:

Tải bộ dữ liệu ASL Alphabet và giải nén vào thư mục raw_data/.

Chạy script tiền xử lý:

Bash

python pc/prepare_dataset.py
Chạy script huấn luyện mô hình (khoảng 15-20 phút):

Bash

python pc/train_model.py
Sau bước này, file model.tflite và biểu đồ huấn luyện sẽ được tạo ra.

3. Cài đặt cho ESP32
Mở file esp32/web/web.ino bằng Arduino IDE.

Chỉnh sửa tên Wifi và Mật khẩu trong code:

C++

const char* ssid = "TEN_WIFI_CUA_BAN";
const char* password = "MAT_KHAU_WIFI";
Kết nối ESP32 với máy tính và nhấn nút Upload.

▶️ Hướng dẫn chạy hệ thống (Usage)
Bước 1: Chạy Server trên máy tính:

Bash

python pc/run_system.py
Màn hình sẽ hiện thông báo: Server đang chạy tại: http://192.168.1.XX:5000

Bước 2: Khởi động ESP32

Cấp nguồn cho ESP32.

Mở Serial Monitor (Baud 115200) để xem địa chỉ IP mà ESP32 nhận được (ví dụ: 192.168.1.73).

Bước 3: Trải nghiệm

Mở trình duyệt (trên máy tính hoặc điện thoại), truy cập vào IP của ESP32.

Đưa tay vào khung camera để hệ thống nhận diện.

📊 Kết quả thực nghiệm (Results)
1. Hiệu suất mô hình
Mô hình đạt độ chính xác ~95% sau 15 epochs.

<p float="left"> <img src="accuracy_chart.png" width="45%" /> <img src="loss_chart.png" width="45%" /> </p>

2. Ma trận nhầm lẫn (Confusion Matrix)
Phân tích các trường hợp nhận diện sai (ví dụ giữa chữ M và N).

3. Giao diện người dùng
Giao diện chờ (trái) và Giao diện khi đang nhận diện (phải).

<p float="left"> <img src="web_ui_overview.jpg" width="45%" /> <img src="web_ui_detection.jpg" width="45%" /> </p>

🐛 Khắc phục lỗi thường gặp (Troubleshooting)
Lỗi Connection Timed Out: Kiểm tra xem PC và ESP32 có bắt chung một mạng Wi-Fi không. Kiểm tra IP của PC có bị đổi không (xem lại log khi chạy run_system.py).

Lỗi Camera không lên: Đảm bảo không có ứng dụng nào khác (Zoom, Teams) đang chiếm dụng Webcam.

ESP32 không kết nối được Wifi: Kiểm tra lại SSID và Password trong file .ino. Chú ý Wifi 2.4GHz (ESP32 không hỗ trợ 5GHz).

👨‍💻 Tác giả
Nguyễn Đào Nam Hải

Trường: Đại học Bách Khoa Hà Nội (HUST)

Lớp: IT3150 - Kỹ thuật máy tính
