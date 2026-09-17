# SPDX-License-Identifier: Apache-2.0
# 软件 SPI 回环 — RA8P1 Titan Mini
# 本工程新增示例；接口依据 docs/peripherals.md 和 platform/machine、platform/openmv。
# 条件: 跳线连接 U18/35 P602 MOSI 与 U18/32 P605 MISO；SCK 使用 U18/33 P603；断开其他器件。
# 适配: SoftSPI 使用空闲 GPIO，退出恢复输入。

from machine import Pin, SoftSPI

sck = Pin("P603")
mosi = Pin("P602")
miso = Pin("P605")
spi = None
try:
    spi = SoftSPI(baudrate=100000, polarity=0, phase=0, sck=sck, mosi=mosi, miso=miso)
    payload = b"OpenMV SoftSPI"
    received = bytearray(len(payload))
    spi.write_readinto(payload, received)
    print("RX:", received)
    assert received == payload, "Check P602 MOSI -> P605 MISO jumper"
    print("SoftSPI loopback OK")
finally:
    if spi is not None:
        spi.deinit()
    for pin in (sck, mosi, miso):
        pin.init(Pin.IN)
