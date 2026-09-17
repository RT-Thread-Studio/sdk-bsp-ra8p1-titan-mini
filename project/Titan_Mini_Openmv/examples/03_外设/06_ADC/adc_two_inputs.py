# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# 双路 ADC 轮询 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/adc.py
# 条件: 模拟输入 A=U18/11 P000、B=U18/13 P001，共地；电压不得超出 0..3.3 V 模拟电源域。
# 适配: 两个输入串行采样，不声称同时采样；构造失败也释放已创建通道。

import time
from machine import Pin, ADC

channels = []
try:
    for name in ("P000", "P001"):
        channels.append(ADC(Pin(name)))
    while True:
        values = [channel.read_u16() for channel in channels]
        print("AN000=%d, AN001=%d" % (values[0], values[1]))
        time.sleep_ms(200)
finally:
    for channel in channels:
        channel.deinit()
