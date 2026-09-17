# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# Pixy v1 AprilTag UART 输出 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/07-Interface-Library/01-Pixy-Emulation/apriltags_pixy_uart_emulation.py
# 条件: H1/3=P801 UART2 TX 接接收方 RX，H1/1 共地；3.3 V TTL；H1 可能未焊排针。USB CDC 保留给 IDE。
# 条件: OV5640 与配套 openmv-vision.bin 已就绪；串口接收端需实现对应协议。
# 条件: 接收端 19200 8N1；呈现 TAG36H11 AprilTag（不是普通 QR 码）。
# 适配: 仅输出 Pixy v1 color-code block：signature=tag.id+8；删除 Servo/DAC 和接收命令。
# 适配: QQVGA 灰度；最多 10 个结果；角度用有符号 int16，其他字段用 uint16。

import csi
import image
import math
import struct
import time
from machine import UART

def pixy_tag_block(tag_id, cx, cy, width, height, rotation):
    angle = int(math.degrees(rotation))
    angle = (angle + 180) % 360 - 180
    values = (int(tag_id) + 8, int(cx), int(cy), int(width), int(height))
    payload = struct.pack("<5Hh", *values, angle)
    checksum = (sum(values) + (angle & 0xFFFF)) & 0xFFFF
    return struct.pack("<HH", 0xAA56, checksum) + payload

camera = csi.CSI()
camera.reset()
camera.pixformat(csi.GRAYSCALE)
camera.framesize(csi.QQVGA)
camera.snapshot(time=2000)
uart = UART(2, 19200, timeout=200, timeout_char=10)
try:
    while True:
        img = camera.snapshot()
        tags = img.find_apriltags(families=image.TAG36H11)
        tags = sorted(tags, key=lambda tag: tag.w * tag.h, reverse=True)[:10]
        packet = bytearray()
        if tags:
            packet.extend(struct.pack("<H", 0xAA55))
            for tag in tags:
                packet.extend(pixy_tag_block(tag.id, tag.cx, tag.cy, tag.w, tag.h, tag.rotation))
                img.draw_rectangle(tag.rect, color=255)
                img.draw_cross((tag.cx, tag.cy), color=255)
        packet.extend(struct.pack("<H", 0))
        if uart.write(packet) != len(packet):
            raise OSError("UART short write")
        uart.flush()
        time.sleep_ms(20)
finally:
    uart.deinit()
