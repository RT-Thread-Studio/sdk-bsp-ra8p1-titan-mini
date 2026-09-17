# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# MAVLink v1 光流遥测 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/07-Interface-Library/02-MAVLink/mavlink_opticalflow.py
# 条件: H1/3=P801 UART2 TX 接接收方 RX，H1/1 共地；3.3 V TTL；H1 可能未焊排针。USB CDC 保留给 IDE。
# 条件: OV5640 与配套 openmv-vision.bin 已就绪；串口接收端需实现对应协议。
# 条件: 接收端 115200 8N1；面向有纹理、光照稳定的平面；本例用于协议台架演示。
# 适配: QQVGA 后 window(64,64)，GRAYSCALE；移除官方 35/53 未标定系数，按 dpix=0.1 像素打包。
# 适配: 距离未知=-1；无 IMU/测距补偿，flow_comp_m_x/y 为占位 0，不声称具备飞行控制精度。
# 适配: 严格 uint8 头/quality、uint64 时间、uint16 CRC；检测置信度低时发质量 0；无心跳/参数/命令服务。

import csi
import struct
import time
from machine import UART

# MAVLink common.xml: https://mavlink.io/en/messages/common.html#OPTICAL_FLOW
SYSTEM_ID = 1
COMPONENT_ID = 0x54
MIN_RESPONSE = 0.1
# 按接收方坐标系检查符号/轴映射；这里保留官方示例 X 反向、Y 正向。
X_SIGN = -1
Y_SIGN = 1

def checksum(data, extra):
    value = 0xFFFF
    for byte in data + bytes((extra,)):
        tmp = byte ^ (value & 0xFF)
        tmp = (tmp ^ (tmp << 4)) & 0xFF
        value = ((value >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4)) & 0xFFFF
    return value

def mavlink_packet(message_id, payload, sequence, extra_crc):
    header = struct.pack("<5B", len(payload), sequence & 0xFF, SYSTEM_ID,
                         COMPONENT_ID, message_id)
    body = header + payload
    return b"\xFE" + body + struct.pack("<H", checksum(body, extra_crc))

def flow_payload(timestamp_us, flow_x, flow_y, quality):
    return struct.pack("<QfffhhBB", timestamp_us, 0.0, 0.0, -1.0,
                       flow_x, flow_y, 0, quality)

camera = csi.CSI()
camera.reset()
camera.pixformat(csi.GRAYSCALE)
camera.framesize(csi.QQVGA)
camera.window((64, 64))
camera.snapshot(time=2000)
# CSI width()/height() 仍表示完整 framesize；复制实际裁剪帧得到相同的 64x64 参考图。
previous = camera.snapshot().copy()
uart = UART(2, 115200, timeout=200, timeout_char=10)
sequence = 0
elapsed_us = 0
last_us = time.ticks_us()
try:
    while True:
        img = camera.snapshot()
        displacement = previous.find_displacement(img)
        previous.draw_image(img)
        now = time.ticks_us()
        elapsed_us += time.ticks_diff(now, last_us)
        last_us = now
        response = max(0.0, min(1.0, displacement.response))
        if response >= MIN_RESPONSE:
            flow_x = max(-32768, min(32767, round(X_SIGN * displacement.x_translation * 10)))
            flow_y = max(-32768, min(32767, round(Y_SIGN * displacement.y_translation * 10)))
            quality = int(response * 255)
        else:
            flow_x, flow_y, quality = 0, 0, 0
        payload = flow_payload(elapsed_us, flow_x, flow_y, quality)
        packet = mavlink_packet(100, payload, sequence, 175)
        sequence = (sequence + 1) & 0xFF
        if uart.write(packet) != len(packet):
            raise OSError("UART short write")
        uart.flush()
        time.sleep_ms(20)
finally:
    uart.deinit()
