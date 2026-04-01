/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * RA8P1 I2S/Audio Driver Header for RT-Thread
 */

#ifndef DRV_AUDIO_H_
#define DRV_AUDIO_H_

#include <rtthread.h>
#include <rtdevice.h>

/* 音频配置常量 */
#define AUDIO_SAMPLE_RATE_HZ            (16000)      /* 16kHz 采样率 */
#define AUDIO_SAMPLES_PER_CHUNK         (1024)       /* 每次传输块大小 */
#define AUDIO_TOTAL_SAMPLES             (4096)       /* 总缓冲区大小 */

/* 音量控制 */
#define DEFAULT_VOLUME                  (70)
#define MIN_VOLUME                      (0)
#define MAX_VOLUME                      (100)

/* MCLK 配置 */
#define MCLK_TARGET_HZ                 (4096000)    /* 4.096 MHz MCLK */

/* 外部变量声明（供其他模块使用） */
extern int16_t   g_audio_src[2][AUDIO_TOTAL_SAMPLES];
extern uint32_t  g_buffer_index;
//extern volatile bool g_data_ready;
//extern volatile bool g_playing;

/* 函数声明 */

/**
 * @brief 配置 I2S MCLK（主时钟）
 * @note 用于 16kHz 采样率，配置 GPT Timer2 输出 4.096 MHz
 */
void configure_i2s_mclk(void);

/**
 * @brief SSI 中断回调函数
 * @param p_args I2S 回调参数
 * @note 当 SSI TX FIFO 变空时调用，用于流式播放
 */
//void i2s_callback(i2s_callback_args_t *p_args);

/**
 * @brief 音频设备驱动初始化
 * @return RT_EOK 表示成功，其他值表示失败
 * @note 在系统启动时自动调用（INIT_DEVICE_EXPORT）
 */
int rt_hw_sound_init(void);

#endif /* DRV_AUDIO_H_ */
