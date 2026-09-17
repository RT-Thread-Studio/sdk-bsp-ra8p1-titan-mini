# SPDX-License-Identifier: Apache-2.0
# 硬件 SPI1 回环 — RA8P1 Titan Mini
# 本工程新增示例；接口依据 docs/peripherals.md 和 platform/machine、platform/openmv。
# 条件: 断开/取消选择所有 SPI 外设，跳线连接 U18/19 P708 MOSI 与 U18/21 P709 MISO；SCK=U18/23 P102。
# 适配: SPI1 也连接相机 BTB；本例只发回环数据，不驱动板载 QSPI Flash。

from machine import SPI

spi = SPI(1, baudrate=1000000, polarity=0, phase=0, bits=8)
try:
    payload = bytes(range(32))
    received = bytearray(len(payload))
    spi.write_readinto(payload, received)
    print("TX:", payload)
    print("RX:", received)
    assert received == payload, "Check P708 MOSI -> P709 MISO jumper"
    print("SPI1 loopback OK")
finally:
    spi.deinit()
