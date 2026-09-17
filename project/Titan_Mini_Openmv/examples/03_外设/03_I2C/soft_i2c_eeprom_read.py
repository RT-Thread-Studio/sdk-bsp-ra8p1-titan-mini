# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# 软件 I2C 读取外接 EEPROM — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/i2c_control.py
# 条件: 外接 3.3 V 24C02，地址 0x50；SCL=U18/7 P601，SDA=U18/12 P604，4.7 kΩ 上拉至 3.3 V，共地。
# 适配: SoftI2C 100 kHz 与独立 GPIO；只读，不修改 EEPROM 内容。

from machine import Pin, SoftI2C

scl = Pin("P601")
sda = Pin("P604")
try:
    i2c = SoftI2C(scl=scl, sda=sda, freq=100000)
    if 0x50 not in i2c.scan():
        raise OSError("24C02 EEPROM not found")
    data = i2c.readfrom_mem(0x50, 0, 256, addrsize=8)
    for offset in range(0, len(data), 16):
        print("%02X: %s" % (offset, " ".join("%02X" % byte for byte in data[offset:offset + 16])))
finally:
    scl.init(Pin.IN)
    sda.init(Pin.IN)
