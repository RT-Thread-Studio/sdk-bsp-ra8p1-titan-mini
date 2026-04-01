/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-18     RT-Thread    the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG              "es8156"
#define DBG_LVL              DBG_INFO
#include <rtdbg.h>

/* ES8156 寄存器定义 */
#define ES8156_REG_00_RESET       0x00
#define ES8156_REG_01_CLK_MANAGE  0x01
#define ES8156_REG_02_MODE_CTRL   0x02
#define ES8156_REG_03_LRCK_DIV_H  0x03
#define ES8156_REG_04_LRCK_DIV_L  0x04
#define ES8156_REG_05_SCLK_DIV    0x05
#define ES8156_REG_08_DAC_PWR     0x08
#define ES8156_REG_09_CLK_MODE    0x09
#define ES8156_REG_0A_ANALOG_1    0x0A
#define ES8156_REG_0B_ANALOG_2    0x0B
#define ES8156_REG_0D_DAC_CTRL    0x0D
#define ES8156_REG_10_OSR         0x10
#define ES8156_REG_11_IF_FMT      0x11
#define ES8156_REG_13_DAC_MUTE    0x13
#define ES8156_REG_14_DAC_VOL     0x14
#define ES8156_REG_18_PWR_CTRL    0x18
#define ES8156_REG_19_PWR_CTRL_2  0x19
#define ES8156_REG_20_ANA_OUT     0x20
#define ES8156_REG_21_ANA_OUT_2   0x21
#define ES8156_REG_22_CP_CTRL     0x22
#define ES8156_REG_23_VDDA_SET    0x23
#define ES8156_REG_24_ANA_TRIM    0x24
#define ES8156_REG_25_BIAS_CTRL   0x25
#define ES8156_REG_FD_CHIP_ID_L   0xFD

/* ES8156 I2C 配置 */
#define ES8156_I2C_BUS   "i2c1"
#define ES8156_I2C_ADDR  0x08     /* ADDR脚接GND=0x08，接VDD=0x09 */

/* ES8156 模拟电压：0=3.3V，1=1.8V */
#define ES8156_VDDA      0

/* ES8156 默认音量：191 = 0dB，范围 0~255，±0.5dB/Step */
#define ES8156_VOL_DEFAULT  191

struct es8156_device
{
    struct rt_i2c_bus_device *i2c_bus;  /* ES8156 I2C 总线句柄 */
    rt_bool_t                inited;
    rt_uint8_t               volume;      /* 当前音量寄存器值 0~255 */
};

static struct es8156_device g_es8156;

/**
 * @brief ES8156 写寄存器
 *
 * @param dev ES8156 设备句柄
 * @param reg 寄存器地址
 * @param val 寄存器值
 * @return rt_err_t 执行结果
 */
rt_err_t es8156_write_reg(rt_uint8_t reg, rt_uint8_t val)
{
    struct rt_i2c_msg msgs;
    rt_uint8_t buf[2] = { reg, val };

    msgs.addr  = ES8156_I2C_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf   = buf;
    msgs.len   = 2;

    if (rt_i2c_transfer(g_es8156.i2c_bus, &msgs, 1) != 1)
    {
        LOG_E("ES8156 write reg 0x%02X = 0x%02X failed", reg, val);
        return -RT_ERROR;
    }
    return RT_EOK;
}

/**
 * @brief ES8156 读寄存器
 *
 * @param reg 寄存器地址
 * @param val 读取的寄存器值
 * @return rt_err_t 执行结果
 */
rt_err_t es8156_read_reg(rt_uint8_t reg, rt_uint8_t *val)
{
    struct rt_i2c_msg msgs[2];

    msgs[0].addr  = ES8156_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    msgs[1].addr  = ES8156_I2C_ADDR;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = val;
    msgs[1].len   = 1;

    if (rt_i2c_transfer(g_es8156.i2c_bus, msgs, 2) != 2)
    {
        LOG_E("ES8156 read reg 0x%02X failed", reg);
        return -RT_ERROR;
    }
    return RT_EOK;
}

/**
 * @brief ES8156 初始化
 *
 * @return rt_err_t 执行结果
 */
rt_err_t es8156_init(void)
{
    rt_uint8_t chip_id = 0;

    /* 读取芯片ID验证 */
    es8156_read_reg(0xFD, &chip_id);
    if (chip_id != 0x81)
    {
        LOG_W("ES8156 chip ID mismatch: 0x%02X (expect 0x81), continue anyway", chip_id);
    }

    /* 模式控制 */
    es8156_write_reg(0x02, 0x04);
    es8156_write_reg(0x13, 0x00);

    /* 模拟部分：接 PA/LOUT，不开 HP 驱动（省功耗） */
    es8156_write_reg(0x20, 0x2A);
    es8156_write_reg(0x21, 0x3C);
    es8156_write_reg(0x22, 0x02);
    es8156_write_reg(0x24, 0x07);
    es8156_write_reg(0x23, 0x40);  /* 3V3: 0x40, 1V8: 0x70 */

    es8156_write_reg(0x0A, 0x01);
    es8156_write_reg(0x0B, 0x01);

    es8156_write_reg(0x06, 0x11);
    es8156_write_reg(0x18, 0x10);

    /*
     * 0x11: 数据格式
     *   bits[6:4] = Format_Len (0x03 = 16bit)
     *   bits[2:0] = Format     (0x00 = NORMAL_I2S)
     * → 0x03<<4 | 0x00 = 0x30
     */
    es8156_write_reg(0x11, 0x30);

    /* 其他必要寄存器 */
    es8156_write_reg(0x0D, 0x14);
    es8156_write_reg(0x18, 0x08);
    es8156_write_reg(0x19, 0x00);

    es8156_write_reg(0x08, 0x3F);

    /* 软件复位并启动 */
    es8156_write_reg(0x00, 0x02);
    es8156_write_reg(0x00, 0x03);

    /* 输出使能 */
    es8156_write_reg(0x25, 0x20);

    /* 验证工作状态：回读 0x0C 应为 0x60 */
    rt_uint8_t state = 0;
    es8156_read_reg(0x0C, &state);
    if (state != 0x60)
        LOG_W("ES8156 state = 0x%02X (expect 0x60)", state);
    else
        LOG_I("ES8156 initialized OK (state=0x60)");

    g_es8156.inited = RT_TRUE;
    return RT_EOK;
}

/**
 * @brief ES8156 静音/取消静音
 *
 * @param mute 是否静音
 */
void es8156_mute(rt_bool_t mute)
{
    es8156_write_reg(0x14, mute ? 0x00 : 0x20);
}

/**
 * @brief ES8156 掉电
 */
void es8156_powerdown(void)
{
    es8156_write_reg(0x14, 0x00);
    es8156_write_reg(0x19, 0x02);
    es8156_write_reg(0x22, 0x02);
    es8156_write_reg(0x25, 0x81);
    es8156_write_reg(0x18, 0x01);
    es8156_write_reg(0x09, 0x02);
    es8156_write_reg(0x09, 0x01);
    es8156_write_reg(0x08, 0x00);
    es8156_write_reg(0x25, 0x87);
    LOG_D("ES8156 powered down");
}

/**
 * @brief ES8156 设置音量
 *
 * @param volume 音量值 (0-255)
 */
void es8156_set_volume(rt_uint8_t volume)
{
    es8156_write_reg(0x14, volume);
    g_es8156.volume = volume;
}

/**
 * @brief ES8156 设备初始化
 *
 * @return rt_err_t 执行结果
 */
rt_err_t es8156_device_init(void)
{
    rt_err_t result = RT_EOK;

    /* 查找 I2C 总线 */
    g_es8156.i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(ES8156_I2C_BUS);
    if (g_es8156.i2c_bus == RT_NULL)
    {
        LOG_E("ES8156 I2C bus %s not found!", ES8156_I2C_BUS);
        return -RT_ERROR;
    }

    /* 初始化 ES8156 */
    // result = es8156_init();
    // if (result != RT_EOK)
    // {
    //     LOG_E("ES8156 init failed!");
    //     return result;
    // }

    /* 设置默认音量 */
    // es8156_set_volume(ES8156_VOL_DEFAULT);

    LOG_I("ES8156 driver initialized");
    return RT_EOK;
}

/**
 * @brief 获取 ES8156 设备句柄
 *
 * @return struct es8156_device* ES8156 设备句柄
 */
struct es8156_device *es8156_get_device(void)
{
    return &g_es8156;
}

/**
 * @brief ES8156 初始化（扩展版本，带初始音量设置）
 *
 * @param init_volume 初始音量值 (0-255)
 * @return rt_err_t 执行结果
 */
rt_err_t es8156_init_ex(rt_uint8_t init_volume)
{
    rt_err_t result;

    result = es8156_init();
    if (result != RT_EOK)
    {
        return result;
    }

    /* 设置初始音量 */
    es8156_set_volume(init_volume);

    return RT_EOK;
}
