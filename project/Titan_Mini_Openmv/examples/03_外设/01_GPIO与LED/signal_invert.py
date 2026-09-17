# SPDX-License-Identifier: Apache-2.0
# Signal 逻辑反相 — RA8P1 Titan Mini
# 本工程新增示例；接口依据 docs/peripherals.md 和 platform/machine、platform/openmv。
# 条件: U18/7=P601 接示波器；或外接低电平点亮 LED，串联限流电阻至 3.3 V。
# 适配: 使用当前固件启用的 machine.Signal。

import time
from machine import Pin, Signal

pin = Pin("P601", Pin.OUT, value=1)
signal = Signal(pin, invert=True)
try:
    while True:
        signal.on()  # 逻辑开，物理低。
        time.sleep_ms(500)
        signal.off()  # 逻辑关，物理高。
        time.sleep_ms(500)
finally:
    signal.off()
    pin.init(Pin.IN)
