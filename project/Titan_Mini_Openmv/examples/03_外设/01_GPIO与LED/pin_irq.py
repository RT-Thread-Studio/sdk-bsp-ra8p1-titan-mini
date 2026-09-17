# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# GPIO 下降沿中断 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/pin_irq.py
# 条件: 外接开关连接 U18/7=P601 与 GND；使用内部上拉。
# 适配: P601 替代 SW；调度器回调只记录事件，主循环消抖并输出。

import time
from machine import Pin, LED

button = Pin("P601", Pin.IN, Pin.PULL_UP)
led = LED(3)
pending = False

def on_falling(pin):
    global pending
    pending = True

last = time.ticks_ms()
try:
    button.irq(handler=on_falling, trigger=Pin.IRQ_FALLING)
    while True:
        if pending:
            pending = False
            now = time.ticks_ms()
            if time.ticks_diff(now, last) >= 50:
                last = now
                led.toggle()
                print("button falling edge")
        time.sleep_ms(10)
finally:
    button.irq(handler=None)
    button.init(Pin.IN, Pin.PULL_NONE)
    led.off()
