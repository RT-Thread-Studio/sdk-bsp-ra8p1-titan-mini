# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2021 Arm Limited. All rights reserved.
# Adapted to Python/OpenMV with raw quantized outputs and bounded candidates.
# YOLOv8n-pose: person boxes and 17 COCO keypoints (multiple people).
# Decode contract: HimaxWiseEyePlus/YOLOv8_on_WE2, commit
# 2483a5ebe99576947ecf7e4564aff9cec93fa373,
# source/use_case/img_yolov8_pose_192/src/UseCaseHandler.cc (Arm, Apache-2.0).
# Copy yolov8n_pose_192_u55_256.tflite to the storage root (/rom alias).
# Model weights have separate notices in the package licenses/ directory.
# Model and license information: models/README.md and models/licenses/.
# Titan Mini：YOLO 姿态识别 + 800x480 RGB565 LCD，多人框、17 关键点和骨架。
# 复用 examples/02_机器学习/00_神经网络/yolov8n_pose.py 的解码器。
# 需要支持 lcd 的固件、配套视觉 BIN、OV5640 和上述姿态模型。
# 将 models/sdcard/ 中同名模型复制到板端存储根目录，无需创建 rom 目录。
# QVGA 画面和识别结果一起等比放大 2 倍居中显示，不再变换关键点坐标。
# LCD 占用 GPT7，不要同时运行同组 PWM 示例。

import board
import csi
import lcd
import math
import ml
import time

MODEL_PATH = "/rom/yolov8n_pose_192_u55_256.tflite"
BACKLIGHT = 80  # 0..100
PERSON_THRESHOLD = 0.4
KEYPOINT_THRESHOLD = 0.5
NMS_THRESHOLD = 0.45

# COCO order: nose, eyes, ears, shoulders, elbows, wrists, hips, knees, ankles.
SKELETON = ((0, 1), (0, 2), (1, 3), (2, 4), (5, 6),
            (5, 7), (7, 9), (6, 8), (8, 10), (5, 11),
            (6, 12), (11, 12), (11, 13), (13, 15), (12, 14), (14, 16))


class YoloV8Pose:
    # This model exports raw DFL, person and keypoint heads, not COCO80 boxes.
    SHAPES = ((1, 144, 64), (1, 576, 64), (1, 36, 1), (1, 756, 51),
              (1, 576, 1), (1, 36, 64), (1, 144, 1))
    SCALES = (0.0743677095, 0.0850540623, 0.1490852833, 0.0513033383,
              0.0935444459, 0.0694025531, 0.1864520013)
    ZEROS = (-72, -64, 107, 5, 110, -64, 116)
    # stride, grid width, bbox output, person output, keypoint anchor offset
    HEADS = ((8, 24, 1, 4, 0), (16, 12, 0, 6, 576), (32, 6, 5, 2, 720))

    def __init__(self, threshold=0.4, nms_threshold=0.45, max_candidates=60,
                 max_people=10):
        if not 0.0 < threshold < 1.0 or not 0.0 <= nms_threshold <= 1.0:
            raise ValueError("Invalid pose threshold")
        if max_candidates < 1 or max_people < 1:
            raise ValueError("Pose limits must be positive")
        self.threshold = threshold
        self.nms_threshold = nms_threshold
        self.max_candidates = max_candidates
        self.max_people = max_people

    def validate(self, model):
        if (len(model.input_shape) != 1
                or tuple(model.input_shape[0]) != (1, 192, 192, 3)
                or model.input_dtype[0] != "b"
                or abs(model.input_scale[0] - 1.0 / 255.0) > 1e-8
                or model.input_zero_point[0] != -128
                or tuple(tuple(s) for s in model.output_shape) != self.SHAPES
                or tuple(model.output_dtype) != ("b",) * 7
                or tuple(model.output_zero_point) != self.ZEROS
                or len(model.output_scale) != 7
                or any(abs(a - b) > 1e-7 for a, b in zip(model.output_scale, self.SCALES))):
            raise ValueError("Use the supplied YOLOv8n-pose 192 U55-256 model")

    @staticmethod
    def sigmoid(value):
        if value >= 0:
            return 1.0 / (1.0 + math.exp(-value))
        value = math.exp(value)
        return value / (1.0 + value)

    @staticmethod
    def dfl(tensor, row, offset, scale):
        # Stable softmax expectation of bins 0..15. Zero point cancels here.
        peak = max(int(tensor[0, row, offset + k]) for k in range(16))
        total = 0.0
        weighted = 0.0
        for k in range(16):
            value = math.exp((int(tensor[0, row, offset + k]) - peak) * scale)
            total += value
            weighted += k * value
        return weighted / total

    @staticmethod
    def iou(a, b):
        intersection = (max(0.0, min(a[2], b[2]) - max(a[0], b[0]))
                        * max(0.0, min(a[3], b[3]) - max(a[1], b[1])))
        union = ((a[2] - a[0]) * (a[3] - a[1])
                 + (b[2] - b[0]) * (b[3] - b[1]) - intersection)
        return intersection / union if union > 0 else 0.0

    def __call__(self, model, inputs, outputs):
        # Keep raw INT8 outputs: no full-size floating-point tensor copies.
        candidates = []
        logit_threshold = math.log(self.threshold / (1.0 - self.threshold))
        for stride, grid, box_output, score_output, base in self.HEADS:
            scale = model.output_scale[score_output]
            zero = model.output_zero_point[score_output]
            for row in range(grid * grid):
                logit = (int(outputs[score_output][0, row, 0]) - zero) * scale
                if logit >= logit_threshold:
                    candidate = (self.sigmoid(logit), row, stride, grid,
                                 box_output, base + row)
                    # Keep the same descending tuple order, including ties,
                    # while bounding storage even on a noisy frame.
                    if len(candidates) < self.max_candidates:
                        candidates.append(candidate)
                    elif candidate > candidates[-1]:
                        candidates[-1] = candidate
                    else:
                        continue
                    candidates.sort(reverse=True)
        # Decode only the strongest candidates and bound Python/NMS work.
        kept = []
        for score, row, stride, grid, box_output, anchor in candidates:
            x = row % grid + 0.5
            y = row // grid + 0.5
            distances = [self.dfl(outputs[box_output], row, k * 16,
                                  model.output_scale[box_output]) for k in range(4)]
            box = ((x - distances[0]) * stride, (y - distances[1]) * stride,
                   (x + distances[2]) * stride, (y + distances[3]) * stride)
            if any(self.iou(box, item[0]) > self.nms_threshold for item in kept):
                continue
            kept.append((box, score, anchor, x, y, stride))
            if len(kept) >= self.max_people:
                break

        rx, ry, rw, rh = inputs[0].roi
        sx, sy = rw / 192.0, rh / 192.0
        scale = model.output_scale[3]
        zero = model.output_zero_point[3]
        results = []
        for box, score, anchor, ax, ay, stride in kept:
            x1, y1 = max(0.0, box[0]), max(0.0, box[1])
            x2, y2 = min(192.0, box[2]), min(192.0, box[3])
            if x2 <= x1 or y2 <= y1:
                continue
            left, top = rx + int(x1 * sx), ry + int(y1 * sy)
            right, bottom = rx + int(x2 * sx), ry + int(y2 * sy)
            if right <= left or bottom <= top:
                continue
            points = []
            for k in range(17):
                px = (int(outputs[3][0, anchor, k * 3]) - zero) * scale
                py = (int(outputs[3][0, anchor, k * 3 + 1]) - zero) * scale
                confidence = self.sigmoid(
                    (int(outputs[3][0, anchor, k * 3 + 2]) - zero) * scale)
                px = (px * 2.0 + ax - 0.5) * stride
                py = (py * 2.0 + ay - 0.5) * stride
                # Hide out-of-frame joints instead of drawing false edge points.
                if not (0.0 <= px < 192.0 and 0.0 <= py < 192.0):
                    confidence = 0.0
                points.append((rx + int(px * sx), ry + int(py * sy), confidence))
            results.append(((left, top, right - left, bottom - top), score, points))
        return results



def main():
    camera = None
    lcd_ready = False
    previous_preview = board.preview(False)
    try:
        # 先加载模型，保持与原姿态示例完全相同的输入/原始量化输出契约。
        decoder = YoloV8Pose(PERSON_THRESHOLD, NMS_THRESHOLD)
        model = ml.Model(MODEL_PATH, postprocess=decoder)
        decoder.validate(model)
        print(model)

        lcd.init()
        lcd_ready = True
        lcd.backlight(BACKLIGHT)
        camera = csi.CSI()
        camera.reset()
        camera.pixformat(csi.RGB565)
        camera.framesize(csi.QVGA)
        camera.snapshot(time=2000)

        # 完整流程帧率，包含采集、预处理、推理、后处理、绘图及 LCD 换帧。
        # LCD 显示上一窗口统计值，第一个约 1 秒窗口结束前显示 0。
        fps_start = time.ticks_us()
        frames = 0
        app_fps = 0.0
        while True:
            img = camera.snapshot()
            people = model.predict([img])
            for rect, score, points in people:
                img.draw_rectangle(rect, color=(0, 255, 0), thickness=2)
                img.draw_string((rect[0], max(0, rect[1] - 10)),
                                'person %.2f' % score, color=(0, 255, 0))
                for a, b in SKELETON:
                    if points[a][2] >= KEYPOINT_THRESHOLD and points[b][2] >= KEYPOINT_THRESHOLD:
                        img.draw_line((points[a][0], points[a][1], points[b][0], points[b][1]),
                                      color=(0, 255, 255), thickness=2)
                for x, y, confidence in points:
                    if confidence >= KEYPOINT_THRESHOLD:
                        img.draw_circle((x, y, 2), color=(255, 255, 0), fill=True)

            # 标注均在相机图像坐标系完成，LCD 对图像和标注一起缩放。
            img.draw_rectangle((0, 0, img.width(), 14), color=(0, 0, 0), fill=True)
            img.draw_string((4, 3), 'People: %d  App FPS: %.1f' % (len(people), app_fps),
                            color=(255, 255, 255))
            lcd.display(img, x=80, y=0, x_scale=2, y_scale=2)
            frames += 1
            now = time.ticks_us()
            elapsed = time.ticks_diff(now, fps_start)
            if elapsed >= 1_000_000:
                app_fps = frames * 1_000_000 / elapsed
                fps_start = now
                frames = 0
    finally:
        try:
            if camera is not None:
                camera.shutdown(True)
        finally:
            try:
                if lcd_ready:
                    lcd.deinit()
            finally:
                board.preview(previous_preview)


if __name__ == '__main__':
    main()
