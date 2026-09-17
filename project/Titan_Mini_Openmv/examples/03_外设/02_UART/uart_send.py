# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# UART2 周期发送 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/uart_control.py
# 条件: UART2: H1/3=P801 TX 接外部 RX，H1/2=P802 RX 接外部 TX，H1/1 共地；3.3 V TTL、115200 8N1，H1 可能未焊排针。
# 适配: 使用 UART2；UART1 为 msh，USB CDC 为 IDE，均不占用。

import time
from machine import UART

uart = UART(2, 115200, bits=8, parity=None, stop=1,
            timeout=100, timeout_char=10, rxbuf=256)
try:
    count = 0
    while True:
        data = ("Titan Mini %d\r\n" % count).encode()
        sent = uart.write(data)
        if sent != len(data):
            raise OSError("UART short write")
        uart.flush()
        count += 1
        time.sleep_ms(1000)
finally:
    uart.deinit()
