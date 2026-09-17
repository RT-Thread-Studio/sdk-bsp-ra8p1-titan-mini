# SPDX-License-Identifier: Apache-2.0
# UART2 回环 — RA8P1 Titan Mini
# 本工程新增示例；接口依据 docs/peripherals.md 和 platform/machine、platform/openmv。
# 条件: 断开 H1 外部设备，只用跳线连接 H1/3=P801 TX 与 H1/2=P802 RX；H1 可能未焊接。
# 适配: 依照 examples/BOARD.md 的 UART2 引脚说明接线，使用短数据读回。

from machine import UART

uart = UART(2, 115200, timeout=200, timeout_char=10, rxbuf=256)
try:
    payload = b"OpenMV Titan Mini UART2 loopback\r\n"
    if uart.write(payload) != len(payload):
        raise OSError("UART short write")
    uart.flush()
    received = uart.read(len(payload))
    print("TX:", payload)
    print("RX:", received)
    assert received == payload, "Check P801 TX -> P802 RX jumper"
    print("UART2 loopback OK")
finally:
    uart.deinit()
