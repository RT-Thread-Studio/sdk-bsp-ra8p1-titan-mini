# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# GPIO 输出与翻转 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/pin_control.py
# 条件: U18/7=P601，连接示波器或串有限流电阻的 LED 至 GND；3.3 V 电平。
# 适配: 改为 P601 输出演示，不使用其他板的 P0/P1 别名。

import time
from machine import Pin

pin = Pin("P601", Pin.OUT, value=0)
try:
    while True:
        pin.on()
        time.sleep_ms(500)
        pin.off()
        time.sleep_ms(500)
        pin.toggle()
        time.sleep_ms(500)
finally:
    pin.off()
    pin.init(Pin.IN)
