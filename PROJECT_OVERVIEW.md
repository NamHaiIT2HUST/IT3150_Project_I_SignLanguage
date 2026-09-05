# Mô tả dự án: Hệ thống Nhận diện Ngôn ngữ Ký hiệu (ASL) — Kiến trúc Edge AI

## 1. Bối cảnh & mục tiêu

Đồ án PROJECT I (Kỹ thuật Máy tính, HUST) xây dựng một thiết bị **AI + IoT** giúp chuyển cử chỉ tay theo
ngôn ngữ ký hiệu Mỹ (ASL) thành chữ viết theo thời gian thực, hướng tới hỗ trợ giao tiếp cho người khiếm
thính. Bài toán cụ thể: nhận diện 24 chữ cái tĩnh trong bảng ASL (bỏ J và Z vì hai ký hiệu này cần chuyển
động, không nhận diện được từ một khung ảnh tĩnh).

Dự án đã trải qua 2 vòng thiết kế kiến trúc:

- **v1** — đã hoàn thành, đóng gói thành báo cáo nộp (`docs/HaiNDN_20235321_Report_ProjectI.pdf`), lưu tại
  git tag `v1-pc-server-esp32-display`.
- **v2** — bản redesign hiện tại, chuyển sang mô hình **Edge AI** đúng nghĩa: suy luận AI chạy ngay trên
  thiết bị nhúng thay vì trên máy tính.

## 2. Vấn đề của kiến trúc v1

Ở v1, vai trò hai thiết bị bị đảo ngược so với năng lực thực của chúng:

- **PC** (máy tính có CPU/RAM mạnh) chỉ làm việc nền: mở webcam bằng OpenCV, chạy suy luận CNN qua
  TensorFlow Lite, rồi gửi HTTP GET một ký tự sang ESP32.
- **ESP32** (vi điều khiển yếu, không có camera) lại phải tự host toàn bộ web server (`WebServer.h`), tự
  phục vụ HTML/CSS/JS nhúng cứng trong firmware (`web_interface.h`) để hiển thị giao diện cho người dùng
  qua trình duyệt.

Hệ quả: thiết bị yếu gánh việc nặng (phục vụ web app), máy khỏe chỉ làm việc nhẹ (gửi 1 ký tự), đồng thời
toàn bộ hệ thống vẫn phụ thuộc PC — không có gì thực sự "biên" (edge) trong pipeline AI.

## 3. Kiến trúc v2: Edge AI

```
[ESP32-S3-CAM] — thiết bị biên (edge node)         [PC] — server trung tâm
  • Camera OV2640 chụp ảnh grayscale                  • Nhận POST /predict {char, confidence}
  • Crop ROI giữa khung hình, resize 48x48             từ ESP32-S3-CAM
  • Suy luận CNN (TensorFlow Lite Micro, int8)  --->  • Áp dụng logic "giữ ổn định ≥0.8s mới
    NGAY TRÊN CHIP — không cần PC lúc chạy               ghi vào lịch sử" để lọc nhiễu rung tay
  • Gửi kết quả JSON nhỏ qua HTTP POST                • Host dashboard web (Flask) cho trình
  • KHÔNG còn host web server                           duyệt người dùng theo dõi
```

Nguyên tắc thiết kế: **mỗi thiết bị làm đúng việc phù hợp với năng lực của nó** — thiết bị biên (ESP32-S3-CAM)
chỉ cảm biến + suy luận cục bộ + báo cáo kết quả gọn nhẹ; máy chủ trung tâm (PC) lo việc nặng hơn về hiển
thị/tổng hợp/lưu trữ mà nó vốn mạnh hơn nhiều.

### Vì sao cần đổi phần cứng ESP32

Board ESP32 gốc dùng trong v1 không có camera, không thể tự chụp ảnh nên không thể tự suy luận on-device.
Kiến trúc v2 yêu cầu nâng cấp lên **ESP32-S3-CAM** (khuyến nghị Freenove ESP32-S3-WROOM CAM có PSRAM): chip
S3 có tập lệnh vector (`esp-nn`) tăng tốc suy luận CNN nhanh hơn 1-2 bậc độ lớn so với ESP32 gốc, và PSRAM
tách riêng cho khung hình camera giúp dành trọn SRAM nội cho vùng nhớ suy luận (tensor arena) — giảm rủi ro
tràn RAM khi phải chạy đồng thời WiFi + camera + CNN trên một chip nhỏ.

### Vì sao model phải nhỏ lại và lượng tử hoá (quantize)

CNN gốc ở v1 (input 64×64, ~350K tham số, float32, ~1.4MB) không thể chạy trên vi điều khiển: quá nặng cho
bộ nhớ và không tương thích thẳng với TensorFlow Lite Micro (bản TFLite dành cho thiết bị nhúng). Ở v2, CNN
được thu gọn (input 48×48, filter 16/32/32, `padding=same` để tránh tensor trung gian phình to) và lượng tử
hoá **int8 toàn phần** bằng representative dataset. Kết quả đo thực tế: model giảm còn **96.8KB**, val
accuracy **~86%** trên tập Kaggle rút gọn hiện có trong repo (~100 ảnh/lớp).

## 4. Rủi ro kỹ thuật đã xác định

1. **Domain shift dữ liệu**: model train trên ảnh Kaggle `asl_alphabet_train` (chụp bằng thiết bị khác)
   trong khi lúc suy luận thật lại dùng ảnh từ camera OV2640 trên ESP32-S3-CAM — phân phối pixel/ánh
   sáng/góc chụp khác nhau nhiều khả năng làm giảm accuracy đáng kể. Vì vậy dự án có sẵn công cụ thu thập
   dữ liệu hiệu chỉnh trực tiếp từ camera thật (`esp32/tools/capture_mode.ino` +
   `pc/collect_calibration_set.py`) để fine-tune trước khi coi model "dùng được" trên thiết bị thật.
2. **Giới hạn bộ nhớ nhúng**: suy luận trên MCU đòi hỏi tính toán chính xác kích thước vùng nhớ tạm (tensor
   arena) — khác hẳn việc chạy TFLite trên PC chỉ cần `interpreter.allocate_tensors()`.
3. **Tốc độ khung hình (FPS) thấp hơn PC**: v1 đạt 25-30 FPS nhờ chạy trên CPU máy tính; trên ESP32-S3 sau
   khi tối ưu, FPS thực tế dự kiến thấp hơn đáng kể — cần đo đạc thật khi có phần cứng thay vì ước lượng.
4. **Phụ thuộc phần cứng mới**: toàn bộ phần suy luận on-device (firmware `edge_inference.ino`) mới chỉ
   được viết theo tài liệu tham khảo, **chưa flash và kiểm thử trên board thật** vì tại thời điểm redesign
   chưa có ESP32-S3-CAM — cần xác nhận lại pin mapping camera và đo thực nghiệm khi có board.

## 5. Cấu trúc thư mục & vai trò từng thành phần

```
esp32/
├── edge_inference/       # Firmware CHÍNH THỨC (v2)
│   ├── edge_inference.ino    # Vòng lặp chính: chụp ảnh -> tiền xử lý -> suy luận TFLite Micro -> POST kết quả
│   ├── camera_pins.h         # Pin mapping camera OV2640 (cần đối chiếu với board thật)
│   ├── roi_config.h          # Kích thước khung hình/ROI/input model dùng chung
│   └── model_data.h          # Model int8 nhúng dạng mảng C (sinh tự động, không sửa tay)
├── tools/capture_mode.ino    # Sketch tạm: phục vụ thu thập dữ liệu hiệu chỉnh từ camera thật
└── legacy_v1/                 # Firmware kiến trúc v1 (ESP32 tự host web server) — lưu tham khảo

pc/
├── prepare_dataset.py     # Resize ảnh Kaggle về 48x48
├── train_model.py         # Train CNN gọn + quantize int8 -> model.tflite
├── convert_to_c_array.py  # model.tflite -> esp32/edge_inference/model_data.h
├── collect_calibration_set.py  # Thu thập ảnh hiệu chỉnh qua capture_mode.ino
├── server.py               # SERVER TRUNG TÂM v2: nhận /predict, host dashboard, KHÔNG chạy AI nữa
├── templates/, static/     # Giao diện dashboard
├── model/                  # model.tflite (int8) + labels.json
└── legacy_v1/               # Server v1 (PC làm inference) — lưu tham khảo
```

## 6. Trạng thái hiện tại & việc còn lại

Đã hoàn thành và kiểm thử được (không cần phần cứng mới):
- Pipeline train + quantize (đã chạy thật, có số liệu accuracy/kích thước model thật).
- PC server trung tâm + dashboard (đã test end-to-end bằng cách giả lập request từ ESP32).
- Tái cấu trúc thư mục, bảo tồn kiến trúc v1 qua git tag để tham khảo/so sánh.

Đã viết nhưng chờ phần cứng để kiểm thử:
- Firmware `edge_inference.ino` chạy on-device trên ESP32-S3-CAM.
- Quy trình thu thập dữ liệu hiệu chỉnh từ camera thật.
- Đo đạc FPS, mức dùng bộ nhớ (tensor arena), và accuracy thực tế trên thiết bị thật — các số liệu này cần
  bổ sung vào báo cáo sau khi có board ESP32-S3-CAM.
