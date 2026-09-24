# SPDX-License-Identifier: MIT
# Titan Mini OpenMV 示例：红色色块追踪并显示到 RGB LCD
# 需要支持 lcd 的固件、800x480 RGB565 屏和已连接的相机，无需模型文件。
# QVGA 图像等比放大 2 倍，在屏幕中央显示为 640x480。

import board
import csi
import lcd
import time

BACKLIGHT = 80  # 背光亮度：0~100
# LAB 阈值：(L 最小值, L 最大值, A 最小值, A 最大值, B 最小值, B 最大值)。
# 根据光照和目标颜色调整；可先在 IDE 阈值编辑器中选取目标的阈值。
RED_THRESHOLD = (30, 100, 15, 127, 15, 127)
PIXELS_THRESHOLD = 200
AREA_THRESHOLD = 200

camera = None
previous_preview = board.preview(False)
try:
    lcd.init()
    lcd.backlight(BACKLIGHT)

    camera = csi.CSI()
    camera.reset()
    camera.pixformat(csi.RGB565)
    camera.framesize(csi.QVGA)
    camera.snapshot(time=2000)
    # 自动调整稳定后锁定增益和白平衡，避免颜色阈值随画面变化。
    camera.auto_gain(False)
    camera.auto_whitebal(False)
    # 统计完整循环：采集、识别、叠加、LCD 绘制和等待换帧均计入。
    # 屏幕显示上一统计窗口的结果，首个约 1 秒窗口结束前显示 0。
    fps_start = time.ticks_us()
    frames = 0
    app_fps = 0.0

    while True:
        img = camera.snapshot()
        # 先完成检测，再画框和文字，避免绘制内容被识别为色块。
        blobs = img.find_blobs(
            [RED_THRESHOLD],
            pixels_threshold=PIXELS_THRESHOLD,
            area_threshold=AREA_THRESHOLD,
            merge=True,
        )
        for blob in blobs:
            img.draw_rectangle(blob.rect, color=(0, 255, 0), thickness=2)
            img.draw_cross((blob.cx, blob.cy), color=(255, 255, 255), size=6)
        img.draw_string(
            (4, 4), "Red: %d  App FPS: %.1f" % (len(blobs), app_fps),
            color=(255, 255, 255),
        )
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
            lcd.deinit()
        finally:
            board.preview(previous_preview)
