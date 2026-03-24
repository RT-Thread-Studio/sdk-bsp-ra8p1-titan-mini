/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ES8156_H__
#define __ES8156_H__

#include <rtthread.h>
#include <rtdevice.h>

/**
 * @brief 初始化 ES8156 音频编解码器
 *
 * @return rt_err_t 执行结果
 */
rt_err_t es8156_device_init(void);

/**
 * @brief 获取 ES8156 设备句柄
 *
 * @return struct es8156_device* ES8156 设备句柄
 */
struct es8156_device *es8156_get_device(void);

/**
 * @brief ES8156 设置音量
 *
 * @param volume 音量值 (0-255)
 */
void es8156_set_volume(rt_uint8_t volume);

/**
 * @brief ES8156 静音/取消静音
 *
 * @param mute 是否静音
 */
void es8156_mute(rt_bool_t mute);

/**
 * @brief ES8156 掉电
 */
void es8156_powerdown(void);

#endif /* __ES8156_H__ */
