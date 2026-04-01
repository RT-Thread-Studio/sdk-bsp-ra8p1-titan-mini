/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2024-03-11     kurisaw       first version
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>

/* 配置 LED 灯引脚 */
#define LED_PIN_R   BSP_IO_PORT_01_PIN_09

void hal_entry(void)
{
    rt_kprintf("\nHello RT-Thread!\n");
    rt_kprintf("============================================================\n");
    rt_kprintf("This example project is a Titan Board Mini template routine!\n");
    rt_kprintf("============================================================\n");

    rt_pin_mode(LED_PIN_R, PIN_MODE_OUTPUT);

    while(1)
    {
        rt_pin_write(LED_PIN_R, PIN_LOW);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN_R, PIN_HIGH);
        rt_thread_mdelay(500);
    }
}
