# SPDX-License-Identifier: Apache-2.0
# PWM 脉宽测量 — RA8P1 Titan Mini
# 本工程新增示例；接口依据 docs/peripherals.md 和 platform/machine、platform/openmv。
# 条件: 仅连接 U18/7=P601 到 U18/12=P604；前者输出 PWM，后者输入，勿同时运行其他占针脚示例。
# 适配: time_pulse_us 是轮询测量，适合演示；不作为精准计量。

import time
from machine import Pin, PWM, time_pulse_us

input_pin = Pin("P604", Pin.IN)
pwm = PWM(Pin("P601"), freq=1000, duty_u16=32768)
try:
    while True:
        width = time_pulse_us(input_pin, 1, 10000)
        if width >= 0:
            print("high pulse: %d us (approximately 500 us)" % width)
        else:
            print("No pulse: check P601 -> P604 jumper; code", width)
        time.sleep_ms(500)
finally:
    pwm.deinit()
