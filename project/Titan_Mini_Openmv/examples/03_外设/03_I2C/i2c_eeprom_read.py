# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# 硬件 I2C 读取外接 EEPROM — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/i2c_control.py
# 条件: 外接支持 3.3 V/400 kHz 的 24C02，A0/A1/A2 接地，地址 0x50；SCL=U18/28，SDA=U18/27，共地，合适上拉。
# 适配: 使用 I2C0；读取 8 位地址 EEPROM；不会写入 EEPROM 或相机寄存器。

from machine import I2C

i2c = I2C(0)
if 0x50 not in i2c.scan():
    raise OSError("Connect a 24C02 EEPROM at I2C0 address 0x50")
data = i2c.readfrom_mem(0x50, 0, 256, addrsize=8)
for offset in range(0, len(data), 16):
    print("%02X: %s" % (offset, " ".join("%02X" % byte for byte in data[offset:offset + 16])))
