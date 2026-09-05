# Kiến trúc v1 (đã lưu trữ)

Các file trong thư mục này là mã nguồn PC của kiến trúc **v1**: PC chạy webcam + suy luận TensorFlow Lite,
gửi kết quả qua HTTP GET tới ESP32 (ESP32 tự host web server hiển thị). Đây là kiến trúc được mô tả trong
báo cáo `docs/HaiNDN_20235321_Report_ProjectI.pdf` và được đánh dấu bằng git tag `v1-pc-server-esp32-display`.

Kiến trúc mới (v2 - Edge AI, xem `pc/server.py` và `esp32/edge_inference/`) đảo lại vai trò: ESP32-S3-CAM
tự suy luận on-device, PC chỉ làm server trung tâm host dashboard. Các file ở đây không còn được dùng trong
luồng chạy chính thức, giữ lại để tham khảo/so sánh.

- `app.py`, `run_system.py`, `realtime_detect.py`, `video_server.py`: 3 biến thể server Flask + 1 bản desktop
  (`cv2.imshow`) của pipeline v1, chạy được độc lập với `python pc/legacy_v1/<file>.py` (cần đổi `cwd` về `pc/`
  hoặc sửa lại đường dẫn `model/model.tflite` cho khớp).
