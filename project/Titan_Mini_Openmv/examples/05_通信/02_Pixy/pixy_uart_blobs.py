# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# Pixy v1 普通色块 UART 输出 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/07-Interface-Library/01-Pixy-Emulation/pixy_uart_emulation.py
# 条件: H1/3=P801 UART2 TX 接接收方 RX，H1/1 共地；3.3 V TTL；H1 可能未焊排针。USB CDC 保留给 IDE。
# 条件: OV5640 与配套 openmv-vision.bin 已就绪；串口接收端需实现对应协议。
# 条件: 接收端 19200 8N1；用 IDE 阈值编辑器调整 LAB_THRESHOLDS。
# 适配: 仅实现 Pixy v1 normal block 单向输出，非完整 Pixy 双向模拟。
# 适配: 删除 Servo/DAC、下行命令状态机与颜色编码组合；每帧最多 10 个块；协议标记/校验使用无符号 16 位。

import csi
import struct
import time
from machine import UART

LAB_THRESHOLDS = [(30, 100, 15, 127, 15, 127), (30, 100, -64, -8, -32, 32)]
MAX_BLOCKS = 10

def pixy_normal_block(signature, cx, cy, width, height):
    values = (int(signature), int(cx), int(cy), int(width), int(height))
    payload = struct.pack("<5H", *values)
    return struct.pack("<HH", 0xAA55, sum(values) & 0xFFFF) + payload

camera = csi.CSI()
camera.reset()
camera.pixformat(csi.RGB565)
camera.framesize(csi.QVGA)
camera.snapshot(time=2000)
camera.auto_gain(False)
camera.auto_whitebal(False)
uart = UART(2, 19200, timeout=200, timeout_char=10)
try:
    while True:
        img = camera.snapshot()
        blobs = img.find_blobs(LAB_THRESHOLDS, pixels_threshold=100,
                               area_threshold=100, merge=False)
        blobs = sorted(blobs, key=lambda blob: blob.w * blob.h, reverse=True)[:MAX_BLOCKS]
        packet = bytearray()
        if blobs:
            packet.extend(struct.pack("<H", 0xAA55))
            for blob in blobs:
                signature = 1 if blob.code & 1 else 2
                packet.extend(pixy_normal_block(signature, blob.cx, blob.cy, blob.w, blob.h))
                img.draw_rectangle(blob.rect)
                img.draw_cross((blob.cx, blob.cy))
        packet.extend(struct.pack("<H", 0))
        if uart.write(packet) != len(packet):
            raise OSError("UART short write")
        uart.flush()
        time.sleep_ms(20)
finally:
    uart.deinit()
