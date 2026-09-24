# SPDX-License-Identifier: MIT
# Titan Mini OpenMV 示例：RGB LCD 相机预览
# 需要支持 lcd 的固件、800x480 RGB565 屏和已连接的相机。
# 将 QVGA 图像等比放大 2 倍，左右各留 80 像素黑边，不拉伸图像。

import board
import csi
import lcd
import time

BACKLIGHT = 80  # 背光亮度：0~100

camera = None
# 关闭 IDE 图像预览，减少额外的 JPEG 编码和 USB 传输。
previous_preview = board.preview(False)
try:
    lcd.init()
    lcd.backlight(BACKLIGHT)

    camera = csi.CSI()
    camera.reset()
    camera.pixformat(csi.RGB565)
    camera.framesize(csi.QVGA)
    camera.snapshot(time=2000)  # 等待曝光和白平衡稳定。
    # LCD 显示返回后才计为完成一帧，包含缩放绘制及等待换帧的时间。
    # 显示上一统计窗口的完整循环帧率，首个窗口结束前显示 0。
    fps_start = time.ticks_us()
    frames = 0
    app_fps = 0.0

    while True:
        img = camera.snapshot()
        img.draw_string((4, 4), "App FPS: %.1f" % app_fps, color=(255, 255, 255))
        lcd.display(img, x=80, y=0, x_scale=2, y_scale=2)
        frames += 1
        now = time.ticks_us()
        elapsed = time.ticks_diff(now, fps_start)
        if elapsed >= 1_000_000:
            app_fps = frames * 1_000_000 / elapsed
            fps_start = now
            frames = 0
finally:
    # 停止采集；即使清理相机失败，也释放 LCD 并恢复 IDE 预览状态。
    try:
        if camera is not None:
            camera.shutdown(True)
    finally:
        try:
            lcd.deinit()
        finally:
            board.preview(previous_preview)
