# SPDX-License-Identifier: Apache-2.0
# 软件 I2C 扫描 — RA8P1 Titan Mini
# 本工程新增示例；接口依据 docs/peripherals.md 和 platform/machine、platform/openmv。
# 条件: 外接 I2C 器件：SCL=U18/7 P601，SDA=U18/12 P604，共地；两线各约 4.7 kΩ 上拉至 3.3 V。
# 适配: 使用空闲 GPIO，避免重配硬件 I2C/相机针脚；SoftI2C 无 deinit，最后恢复输入。

from machine import Pin, SoftI2C

scl = Pin("P601")
sda = Pin("P604")
try:
    i2c = SoftI2C(scl=scl, sda=sda, freq=100000)
    print("SoftI2C:", ["0x%02X" % address for address in i2c.scan()])
finally:
    scl.init(Pin.IN)
    sda.init(Pin.IN)
