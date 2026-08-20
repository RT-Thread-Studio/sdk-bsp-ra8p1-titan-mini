/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-06-24     kurisaW       first version
 */

#ifndef __RPMSG_FRAME_H__
#define __RPMSG_FRAME_H__

#include <rtthread.h>

#define RPMSG_M85_TO_M33_TYPE_CONTROL      (0U)
#define RPMSG_M85_TO_M33_TYPE_FACE_DETECT  (1U)

#define RPMSG_M33_TO_M85_TYPE_CONTROL      (0U)
#define RPMSG_M33_TO_M85_TYPE_READY        (1U)

typedef struct m33_to_m85_msg
{
    rt_uint32_t index;
    rt_uint32_t type;
    float left_rear_vel;
    float right_rear_vel;
    float front_steering_angle;
    rt_uint32_t checksum;
} m33_to_m85_msg_t;

typedef struct m85_to_m33_msg
{
    rt_uint32_t index;
    rt_uint32_t type;
    float vehicle_speed;
    float steering_angle;
    rt_uint32_t face_count;
    rt_int16_t face_x1;
    rt_int16_t face_y1;
    rt_int16_t face_x2;
    rt_int16_t face_y2;
    float face_score;
    rt_uint32_t checksum;
} m85_to_m33_msg_t;

rt_uint32_t rpmsg_frame_calc_checksum(const void *msg, rt_size_t size);
rt_err_t rpmsg_frame_verify_checksum(const void *msg, rt_size_t size, rt_uint32_t received_checksum);
void rpmsg_frame_print_m33_to_m85(const m33_to_m85_msg_t *msg, const char *direction);
void rpmsg_frame_print_m85_to_m33(const m85_to_m33_msg_t *msg, const char *direction);

#endif /* __RPMSG_FRAME_H__ */