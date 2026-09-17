# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# PWM 占空比渐变 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/pwm_control.py
# 条件: U18/7=P601(GPT6A)，接示波器或串限流电阻的 LED；使用 3.3 V 电平。
# 适配: 单个 GPT6 输出；不使用官方共享计数器的双路配置。

import time
from machine import Pin, PWM

pwm = PWM(Pin("P601"), freq=1000, duty_u16=0)
try:
    while True:
        for duty in range(0, 65536, 1024):
            pwm.duty_u16(duty)
            time.sleep_ms(20)
        for duty in range(65535, -1, -1024):
            pwm.duty_u16(duty)
            time.sleep_ms(20)
finally:
    pwm.deinit()
