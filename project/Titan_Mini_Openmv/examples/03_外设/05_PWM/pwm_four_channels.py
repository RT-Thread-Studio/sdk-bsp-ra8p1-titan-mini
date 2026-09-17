# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# 四组独立 PWM — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/pwm_control.py
# 条件: P601=U18/7(GPT6)、P603=/33(GPT7)、P605=/32(GPT8)、P715=/38(GPT12)；接示波器。
# 适配: 每组只用一个引脚；GPT7 A/B、GPT8 A/B、GPT12 A/B 不可同时占用。

import time
from machine import Pin, PWM

outputs = []
try:
    for pin, frequency in (("P601", 1000), ("P603", 2000), ("P605", 4000), ("P715", 500)):
        outputs.append(PWM(Pin(pin), freq=frequency, duty_u16=32768))
    while True:
        time.sleep_ms(1000)
finally:
    for output in outputs:
        output.deinit()
