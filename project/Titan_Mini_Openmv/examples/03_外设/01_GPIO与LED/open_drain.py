# SPDX-License-Identifier: Apache-2.0
# 开漏输出 — RA8P1 Titan Mini
# 本工程新增示例；接口依据 docs/peripherals.md 和 platform/machine、platform/openmv。
# 条件: U18/7=P601，通过约 4.7 kΩ 电阻上拉至 3.3 V；用示波器观察；共地。
# 适配: 只输出低电平或释放引脚；退出改为输入。

import time
from machine import Pin

pin = Pin("P601", Pin.OPEN_DRAIN, value=1)
try:
    while True:
        pin.value(0)
        time.sleep_ms(500)
        pin.value(1)
        time.sleep_ms(500)
finally:
    pin.value(1)
    pin.init(Pin.IN)
