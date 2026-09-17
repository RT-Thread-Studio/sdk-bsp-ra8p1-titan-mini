# SPDX-License-Identifier: Apache-2.0
# 板载 LSM6DS3 身份寄存器 — RA8P1 Titan Mini
# 本工程新增示例；接口依据 docs/peripherals.md 和 platform/machine、platform/openmv。
# 条件: 板载 LSM6DS3 位于 I2C1，7 位地址 0x6A；本例仅使用 I2C，不依赖关闭的 imu 模块。
# 适配: 只读 WHO_AM_I(0x0F)，不配置或覆盖器件运行状态。

from machine import I2C

i2c = I2C(1)
address = 0x6A
if address not in i2c.scan():
    raise OSError("LSM6DS3 not found at I2C1 address 0x6A")
identity = i2c.readfrom_mem(address, 0x0F, 1)[0]
print("LSM6DS3 WHO_AM_I = 0x%02X (normally 0x69)" % identity)
