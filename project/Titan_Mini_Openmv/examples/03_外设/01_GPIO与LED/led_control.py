# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# 三路板载 LED 组合 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/led_control.py
# 条件: 无需外接；LED 1/2/3 分别为 P109/P108/P110。
# 适配: 数字编号替代官方字符串名称；退出关闭全部灯。

import time
from machine import LED

leds = [LED(1), LED(2), LED(3)]
try:
    while True:
        for value in range(8):
            for index, led in enumerate(leds):
                if value & (1 << index):
                    led.on()
                else:
                    led.off()
            time.sleep_ms(500)
finally:
    for led in leds:
        led.off()
