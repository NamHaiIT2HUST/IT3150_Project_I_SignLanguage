# Kiến trúc v1 (đã lưu trữ)

Firmware ESP32 của kiến trúc **v1**: ESP32 tự host web server (`WebServer.h`) và phục vụ toàn bộ giao diện
HTML/CSS/JS nhúng trong `web/web_interface.h`, chỉ nhận ký tự đã suy luận từ PC qua `GET /update?char=...`.
Đây là kiến trúc trong báo cáo đã nộp (`docs/HaiNDN_20235321_Report_ProjectI.pdf`), tag git
`v1-pc-server-esp32-display`.

Kiến trúc mới (v2) xem `esp32/edge_inference/` — ESP32-S3-CAM tự suy luận on-device, không còn host web server.
