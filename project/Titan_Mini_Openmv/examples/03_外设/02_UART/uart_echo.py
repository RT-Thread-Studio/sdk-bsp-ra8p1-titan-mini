# SPDX-License-Identifier: Apache-2.0
# UART2 原样回显 — RA8P1 Titan Mini
# 来源: examples/machine_peripherals_smoke.py
# 条件: UART2: H1/3=P801 TX 接外部 RX，H1/2=P802 RX 接外部 TX，H1/1 共地；3.3 V TTL、115200 8N1，H1 可能未焊排针。
# 适配: 使用 readinto() 和预分配缓冲；检查短写；USB 仅显示状态。

import time
from machine import UART

uart = UART(2, 115200, timeout=100, timeout_char=10, rxbuf=512)
buffer = bytearray(128)
view = memoryview(buffer)
print("UART2 echo ready; send bytes from an external TTL serial terminal.")
try:
    while True:
        if uart.any():
            count = uart.readinto(buffer)
            if count:
                if uart.write(view[:count]) != count:
                    raise OSError("UART short write")
                uart.flush()
        time.sleep_ms(1)
finally:
    uart.deinit()
