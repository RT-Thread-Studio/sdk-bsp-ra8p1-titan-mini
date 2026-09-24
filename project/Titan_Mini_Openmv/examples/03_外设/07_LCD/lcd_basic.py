# SPDX-License-Identifier: MIT
# LCD 基础显示 — RA8P1 Titan Mini，800x480 RGB565。
# 条件：连接板载 RGB 接口的屏幕，烧录含 lcd/display 的固件及配套视觉 BIN。
# 内容：色条、文字、基本图形、2 倍缩放及背光调节；无需相机或外部素材。
# 背光 GPT7 由 LCD 管理，不要同时运行占用 GPT7 的 PWM 示例。
# 在 IDE 中停止脚本或按 Ctrl-C，会关闭 LCD 并恢复原来的 IDE 预览设置。

import board
import image
import lcd
import time

WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
COLORS = (
    ("RED", (255, 0, 0)),
    ("GREEN", (0, 255, 0)),
    ("BLUE", (0, 0, 255)),
    ("CYAN", (0, 255, 255)),
    ("MAGENTA", (255, 0, 255)),
    ("YELLOW", (255, 255, 0)),
)

previous_preview = board.preview(False)
try:
    lcd.init()  # 固定 800x480、约 56 Hz、双缓冲。
    lcd.clear()

    # 使用半尺寸画布，显示时放大到 800x480，避免再分配整屏源图像。
    canvas = image.Image(400, 240, image.RGB565)
    canvas.clear()
    canvas.draw_string((8, 8), "Titan Mini RGB LCD", color=WHITE, scale=2)

    for index, (name, color) in enumerate(COLORS):
        x = 8 + index * 64
        canvas.draw_rectangle((x, 46, 60, 50), color=color, fill=True)
        canvas.draw_string((x, 100), name, color=WHITE)

    canvas.draw_rectangle((8, 130, 100, 55), color=WHITE, thickness=2)
    canvas.draw_circle((157, 157, 27), color=(255, 0, 255), thickness=2)
    canvas.draw_line((215, 130, 285, 185), color=(0, 255, 255), thickness=2)
    canvas.draw_rectangle((315, 140, 70, 35), color=(0, 128, 255), fill=True)

    print("LCD: %dx%d RGB565, refresh approximately %d Hz" %
          (lcd.width(), lcd.height(), lcd.refresh()))
    while True:
        for brightness in (25, 50, 75, 100, 75, 50):
            canvas.draw_rectangle((0, 210, 400, 30), color=BLACK, fill=True)
            canvas.draw_string((8, 214), "Backlight: %d%%" % brightness, color=WHITE)
            lcd.display(canvas, scale=2)
            lcd.backlight(brightness)  # 0..100；设为 0 可熄灭背光。
            time.sleep_ms(1000)
finally:
    try:
        lcd.deinit()
    finally:
        board.preview(previous_preview)
