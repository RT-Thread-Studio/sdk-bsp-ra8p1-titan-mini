/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2024-03-11     kurisaW   first version
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>

#define LED_PIN_R    BSP_IO_PORT_01_PIN_09 /* Onboard LED pins */

#define FS_PARTITION_NAME   "filesystem"

void hal_entry(void)
{
    rt_kprintf("\n==================================================\n");
    rt_kprintf("Hello, Titan Board Mini!\n");
    rt_kprintf("==================================================\n");

    extern int fal_init(void);
    extern struct rt_device* fal_mtd_nor_device_create(const char *parition_name);
    fal_init ();

    struct rt_device *mtd_dev = fal_mtd_nor_device_create (FS_PARTITION_NAME);
    if (mtd_dev == NULL)
    {
        rt_kprintf("Can't create a mtd device on '%s' partition.\n", FS_PARTITION_NAME);
    }
    else
    {
        rt_kprintf("Create a mtd device on the %s partition of flash successful.\n", FS_PARTITION_NAME);
    }

    while (1)
    {
        rt_pin_write(LED_PIN_R, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN_R, PIN_LOW);
        rt_thread_mdelay(500);
    }
}
