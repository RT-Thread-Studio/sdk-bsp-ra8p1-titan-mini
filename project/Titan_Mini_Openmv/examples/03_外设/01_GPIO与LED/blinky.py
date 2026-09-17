# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# 板载 LED 闪烁 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/00-HelloWorld/blinky.py
# 条件: 无需外接；停止脚本后熄灭 LED。
# 适配: LED 使用数字编号 1..3；不支持官方 LED_BLUE 字符串。

import time
from machine import LED

led = LED(3)
try:
    while True:
        led.on()
        time.sleep_ms(500)
        led.off()
        time.sleep_ms(500)
finally:
    led.off()
