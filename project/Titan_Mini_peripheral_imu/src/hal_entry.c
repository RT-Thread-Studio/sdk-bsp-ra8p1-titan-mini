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

#define DBG_TAG     "imu"
#define DBG_LVL     DBG_LOG
#include <rtdbg.h>

#define LED_PIN_R   BSP_IO_PORT_01_PIN_09
#define LED_PIN_B   BSP_IO_PORT_01_PIN_10
#define LED_PIN_G   BSP_IO_PORT_01_PIN_08

#define LED_ON  (0)
#define LED_OFF (1)

static const rt_uint8_t _blink_tab[][3] =
{
    {LED_OFF, LED_OFF, LED_OFF},
    {LED_ON,  LED_OFF, LED_OFF},
    {LED_OFF, LED_ON,  LED_OFF},
    {LED_OFF, LED_OFF, LED_ON},
    {LED_ON,  LED_OFF, LED_ON},
    {LED_ON,  LED_ON,  LED_OFF},
    {LED_OFF, LED_ON,  LED_ON},
    {LED_ON,  LED_ON,  LED_ON},
};

void hal_entry(void)
{
    rt_kprintf("\nHello RT-Thread!\n");
    rt_kprintf("==================================================\n");
    rt_kprintf("Titan Mini peripheral IMU demo\n");
    rt_kprintf("  - LED RGB blink runs on boot\n");
    rt_kprintf("  - LSM6DS3TR-C IMU auto-init on boot, commands:\n");
    rt_kprintf("      imu                 read & print one sample\n");
    rt_kprintf("      imu start [ms]      start periodic sampling\n");
    rt_kprintf("      imu stop            stop periodic sampling\n");
    rt_kprintf("==================================================\n");

    unsigned int count = 0;
    unsigned int group_num = sizeof(_blink_tab)/sizeof(_blink_tab[0]);
    unsigned int group_current;

    rt_pin_mode(LED_PIN_R, PIN_MODE_OUTPUT);
    rt_pin_mode(LED_PIN_G, PIN_MODE_OUTPUT);
    rt_pin_mode(LED_PIN_B, PIN_MODE_OUTPUT);
    rt_pin_write(LED_PIN_R, LED_OFF);
    rt_pin_write(LED_PIN_G, LED_OFF);
    rt_pin_write(LED_PIN_B, LED_OFF);

    do
    {
        group_current = count % group_num;

        rt_pin_write(LED_PIN_R, _blink_tab[group_current][0]);
        rt_pin_write(LED_PIN_B, _blink_tab[group_current][1]);
        rt_pin_write(LED_PIN_G, _blink_tab[group_current][2]);

        count++;

        rt_thread_mdelay(500);
    }while(count > 0);
}
