# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# 两路硬件 I2C 扫描 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/50-OpenMV-Boards/52-Alif-Boards/50-Board-Control/i2c_control.py
# 条件: I2C0=U18/28 P410 SCL、U18/27 P409 SDA，与 OV5640 共用；I2C1 为板载 IMU/音频总线，未接 U18。
# 适配: 两路固定 400 kHz/100 ms；仅探测地址，不修改器件寄存器。

from machine import I2C

for bus_id in (0, 1):
    bus = I2C(bus_id)
    print("I2C%d:" % bus_id, ["0x%02X" % address for address in bus.scan()])
# 总线由板级驱动持有；不关闭相机共享的 I2C0。
