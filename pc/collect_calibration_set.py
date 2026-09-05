"""Thu thập bộ dữ liệu hiệu chỉnh (calibration set) trực tiếp từ camera OV2640 trên
ESP32-S3-CAM, để giảm domain shift so với bộ ảnh Kaggle dùng train ban đầu.

Yêu cầu: ESP32-S3-CAM đã nạp firmware esp32/tools/capture_mode.ino và đang chạy
trên cùng mạng WiFi với máy tính này.

Cách dùng:
    python pc/collect_calibration_set.py --esp32-ip 192.168.1.50 --label A --count 60

Mỗi lần chạy thu thập cho 1 nhãn (chữ cái), lặp lại cho từng chữ cái trong 24 lớp.
Nhấn SPACE để chụp 1 ảnh, ESC để dừng sớm.
"""
import argparse
import os

import cv2
import numpy as np
import requests

# Phải khớp CAM_FRAME_WIDTH/HEIGHT/ROI_SIZE/MODEL_INPUT_SIZE trong
# esp32/edge_inference/roi_config.h
CAM_FRAME_WIDTH = 160
CAM_FRAME_HEIGHT = 120
ROI_SIZE = 120
MODEL_INPUT_SIZE = 48

OUT_DIR = "dataset_calibration"


def fetch_frame(esp32_ip: str) -> np.ndarray:
    resp = requests.get(f"http://{esp32_ip}/capture", timeout=3)
    resp.raise_for_status()
    width = int(resp.headers.get("X-Width", CAM_FRAME_WIDTH))
    height = int(resp.headers.get("X-Height", CAM_FRAME_HEIGHT))
    frame = np.frombuffer(resp.content, dtype=np.uint8).reshape((height, width))
    return frame


def center_crop_and_resize(frame: np.ndarray) -> np.ndarray:
    h, w = frame.shape
    x0 = (w - ROI_SIZE) // 2
    y0 = (h - ROI_SIZE) // 2
    roi = frame[y0:y0 + ROI_SIZE, x0:x0 + ROI_SIZE]
    return cv2.resize(roi, (MODEL_INPUT_SIZE, MODEL_INPUT_SIZE))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--esp32-ip", required=True)
    parser.add_argument("--label", required=True, help="Tên chữ cái, vd: A")
    parser.add_argument("--count", type=int, default=60)
    args = parser.parse_args()

    dst_dir = os.path.join(OUT_DIR, args.label)
    os.makedirs(dst_dir, exist_ok=True)
    existing = len(os.listdir(dst_dir))

    print(f"📸 Thu thập nhãn '{args.label}' -> {dst_dir}")
    print("SPACE: chụp | ESC: dừng")

    saved = 0
    while saved < args.count:
        try:
            frame = fetch_frame(args.esp32_ip)
        except requests.RequestException as e:
            print(f"❌ Không lấy được ảnh từ ESP32: {e}")
            break

        preview = cv2.resize(frame, (CAM_FRAME_WIDTH * 3, CAM_FRAME_HEIGHT * 3))
        cv2.putText(preview, f"{args.label}: {saved}/{args.count}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, 255, 2)
        cv2.imshow("Calibration capture (raw preview)", preview)

        key = cv2.waitKey(1) & 0xFF
        if key == 27:  # ESC
            break
        if key == 32:  # SPACE
            sample = center_crop_and_resize(frame)
            out_path = os.path.join(dst_dir, f"{existing + saved}.jpg")
            cv2.imwrite(out_path, sample)
            saved += 1
            print(f"  ✅ Đã lưu {out_path}")

    cv2.destroyAllWindows()
    print(f"🎉 Xong. Đã thu thập {saved} ảnh cho nhãn '{args.label}'.")


if __name__ == "__main__":
    main()
