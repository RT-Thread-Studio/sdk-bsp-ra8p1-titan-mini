# SPDX-License-Identifier: Apache-2.0
# 读取外接 SPI Flash JEDEC ID — RA8P1 Titan Mini
# 本工程新增示例；接口依据 docs/peripherals.md 和 platform/machine、platform/openmv。
# 条件: 外接 3.3 V W25Q 系列 Flash：SCK=P603(U18/33)、MOSI=P602(/35)、MISO=P605(/32)、CS=P601(/7)，共地；WP/HOLD 拉高。
# 适配: 仅查询外接器件 ID；不访问板载存储 Flash，也不擦写。

from machine import Pin, SoftSPI

cs = Pin("P601", Pin.OUT, value=1)
sck, mosi, miso = Pin("P603"), Pin("P602"), Pin("P605")
spi = None
try:
    spi = SoftSPI(baudrate=100000, polarity=0, phase=0, sck=sck, mosi=mosi, miso=miso)
    received = bytearray(4)
    cs.off()
    try:
        spi.write_readinto(b"\x9F\x00\x00\x00", received)
    finally:
        cs.on()
    jedec = received[1:]
    print("JEDEC ID:", " ".join("%02X" % byte for byte in jedec))
    if jedec == b"\x00\x00\x00" or jedec == b"\xFF\xFF\xFF":
        print("Check external Flash power, chip select and wiring.")
finally:
    cs.on()
    if spi is not None:
        spi.deinit()
    for pin in (sck, mosi, miso, cs):
        pin.init(Pin.IN)
