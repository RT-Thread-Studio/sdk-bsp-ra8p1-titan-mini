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
#define DBG_TAG "driver.entry"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define LED_PIN_G    BSP_IO_PORT_01_PIN_08
#define LED_PIN_R    BSP_IO_PORT_01_PIN_09
#define LED_PIN_B    BSP_IO_PORT_01_PIN_10

void hal_entry(void)
{
    rt_kprintf("\n");
    rt_kprintf("============================================================\n");
    rt_kprintf("           Titan Mini Board Driver Test System            \n");
    rt_kprintf("============================================================\n");
    rt_kprintf("\n");
    rt_kprintf("System initialized successfully!\n");
    rt_kprintf("\n");

    rt_pin_mode(BSP_IO_PORT_04_PIN_13, PIN_MODE_OUTPUT);
    rt_pin_write(BSP_IO_PORT_04_PIN_13, PIN_LOW);


    while (1)
    {
        rt_thread_mdelay(500);
    }

}
