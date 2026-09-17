# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# Arduino 串口交互 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/07-Interface-Library/00-Arduino/arduino_uart.py
# 条件: H1/3=P801 UART2 TX 接接收方 RX，H1/1 共地；3.3 V TTL；H1 可能未焊排针。USB CDC 保留给 IDE。
# 条件: 回显还需接收方 TX -> H1/2=P802 RX；5 V Arduino TX 必须电平转换至 3.3 V，不能直接接板。
# 适配: pyb.UART(3) 改为 machine.UART(2)；保留 19200 8N1；不提供 Arduino 固件下载操作。

# 接收端示例（Arduino C++，在 Arduino IDE 中单独编译）：
# void setup() { Serial.begin(19200); }
# void loop() { if (Serial.available()) Serial.write(Serial.read()); }
import time
from machine import UART

uart = UART(2, 19200, timeout=200, timeout_char=10, rxbuf=256)
try:
    while True:
        data = b"Hello World!\n"
        if uart.write(data) != len(data):
            raise OSError("UART short write")
        uart.flush()
        response = uart.read(len(data))
        if response:
            print("Arduino:", response)
        time.sleep_ms(1000)
finally:
    uart.deinit()
