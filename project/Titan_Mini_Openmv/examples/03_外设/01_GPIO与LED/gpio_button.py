# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# 上拉输入控制板载 LED — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/pin_control.py
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/switch_pin.py
# 条件: 外接开关一端接 U18/7=P601，另一端接 GND；不要连接外部电压。
# 适配: 本例使用外接按钮和软件消抖；LED 无 value()，改用 on()/off()。

import time
from machine import Pin, LED

button = Pin("P601", Pin.IN, Pin.PULL_UP)
led = LED(1)
last = 1
try:
    while True:
        value = button.value()
        if value != last:
            time.sleep_ms(20)
            if button.value() == value:
                last = value
                if value:
                    led.off()
                else:
                    led.on()
                print("pressed:", not value)
        time.sleep_ms(10)
finally:
    led.off()
    button.init(Pin.IN, Pin.PULL_NONE)
