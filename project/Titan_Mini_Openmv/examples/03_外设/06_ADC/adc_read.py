# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# 单路 ADC 读取 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/adc.py
# 条件: 模拟信号=U18/11 P000(AN000)，GND 共地；输入范围 0..板载 3.3 V 模拟电源，禁止负压或 5 V。
# 适配: 16 位 ADC_B；3.3 V 为标称参考估算，非校准电压测量。

import time
from machine import Pin, ADC

adc = ADC(Pin("P000"))
try:
    adc.init(sample_ns=2000)
    while True:
        raw = adc.read_u16()
        print("raw=%d, approximate voltage=%.3f V" % (raw, raw * 3.3 / 65535))
        time.sleep_ms(200)
finally:
    adc.deinit()
