# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# PWM 脉宽与舵机信号 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/servo_control.py
# 条件: 信号=U18/7 P601；默认仅接示波器。接舵机时需自供电并共地，确认接受 3.3 V 信号、1..2 ms 行程。
# 适配: 单路 50 Hz PWM，duty_ns() 控制脉宽；舵机供电不能从 GPIO 获取。

import time
from machine import Pin, PWM

pwm = PWM(Pin("P601"), freq=50, duty_ns=1500000)
try:
    while True:
        for pulse_us in (1500, 1000, 1500, 2000):
            pwm.duty_ns(pulse_us * 1000)
            print("pulse:", pulse_us, "us")
            time.sleep_ms(1000)
finally:
    pwm.deinit()
