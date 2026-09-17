# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
# MAVLink v1 AprilTag 目标遥测 — RA8P1 Titan Mini
# 来源: ThirdParty/openmv/scripts/examples/07-Interface-Library/02-MAVLink/mavlink_apriltags_landing_target.py
# 条件: H1/3=P801 UART2 TX 接接收方 RX，H1/1 共地；3.3 V TTL；H1 可能未焊排针。USB CDC 保留给 IDE。
# 条件: OV5640 与配套 openmv-vision.bin 已就绪；串口接收端需实现对应协议。
# 条件: 接收端 115200 8N1；先填写当前镜头、QQVGA 160x120 对应的 FX/FY/CX/CY 和黑色边框实际边长。
# 条件: 本例用于台架接收测试；姿态、坐标轴、时间同步及接收方配置需在真实系统单独验证。
# 适配: 删除官方 OV5650 镜头/传感器数值；未填写标定参数时明确停止，避免输出伪精确距离。
# 适配: 当前 homography_to_pose 以边长 2 的标签为基准，平移乘实际边长/2 后换算为米。
# 适配: 只发 LANDING_TARGET 30 字节基本负载；uint8 头、uint64 时间、uint16 CRC；无控制指令/心跳服务。

import csi
import image
import math
import struct
import time
from machine import UART

# MAVLink common.xml: https://mavlink.io/en/messages/common.html#LANDING_TARGET
# 填入对当前镜头、160x120 输出标定的像素内参，不能直接沿用其他板的镜头参数。
FX_PIXELS = None
FY_PIXELS = None
CX_PIXELS = None
CY_PIXELS = None
# TAG36H11 ID -> 黑色边框边长（米）；按你的打印尺寸修改。
TAG_SIZES_M = {0: 0.165}
SYSTEM_ID = 1
COMPONENT_ID = 0x54

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

def landing_payload(timestamp_us, tag_id, cx, cy, width, height, tx, ty, tz, tag_size):
    angle_x = math.atan((cx - CX_PIXELS) / FX_PIXELS)
    angle_y = math.atan((cy - CY_PIXELS) / FY_PIXELS)
    distance_m = math.sqrt(tx * tx + ty * ty + tz * tz) * tag_size / 2
    size_x = 2 * math.atan(width / (2 * FX_PIXELS))
    size_y = 2 * math.atan(height / (2 * FY_PIXELS))
    # frame=8 是官方例的 BODY_NED；安装方向及接收方角度约定须自行核实。
    return struct.pack("<QfffffBB", timestamp_us, angle_x, angle_y, distance_m,
                       size_x, size_y, tag_id, 8)

if any(value is None for value in (FX_PIXELS, FY_PIXELS, CX_PIXELS, CY_PIXELS)):
    raise ValueError("Set calibrated FX/FY/CX/CY for OV5640 at 160x120 before running")
if FX_PIXELS <= 0 or FY_PIXELS <= 0:
    raise ValueError("Focal lengths must be positive")
for tag_id, size in TAG_SIZES_M.items():
    if not 0 <= tag_id <= 255 or size <= 0:
        raise ValueError("Target IDs must fit uint8, and tag sizes must be positive")

camera = csi.CSI()
camera.reset()
camera.pixformat(csi.GRAYSCALE)
camera.framesize(csi.QQVGA)
camera.snapshot(time=2000)
uart = UART(2, 115200, timeout=200, timeout_char=10)
sequence = 0
elapsed_us = 0
last_us = time.ticks_us()
try:
    while True:
        img = camera.snapshot()
        tags = img.find_apriltags(families=image.TAG36H11, fx=FX_PIXELS,
                                 fy=FY_PIXELS, cx=CX_PIXELS, cy=CY_PIXELS)
        now = time.ticks_us()
        elapsed_us += time.ticks_diff(now, last_us)
        last_us = now
        tags = [tag for tag in tags if tag.id in TAG_SIZES_M]
        if tags:
            tag = max(tags, key=lambda value: value.w * value.h)
            payload = landing_payload(elapsed_us, tag.id, tag.cx, tag.cy, tag.w, tag.h,
                                      tag.x_translation, tag.y_translation,
                                      tag.z_translation, TAG_SIZES_M[tag.id])
            packet = mavlink_packet(149, payload, sequence, 200)
            sequence = (sequence + 1) & 0xFF
            if uart.write(packet) != len(packet):
                raise OSError("UART short write")
            uart.flush()
            img.draw_rectangle(tag.rect, color=255)
            img.draw_cross((tag.cx, tag.cy), color=255)
        # 未找到目标时不发送上一帧坐标。
        time.sleep_ms(20)
finally:
    uart.deinit()
