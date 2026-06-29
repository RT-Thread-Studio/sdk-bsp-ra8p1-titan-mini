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
#if BSP_CFG_DCACHE_ENABLED
 #include "core_cm85.h"
#endif

#include "usb_pcdc.h"

#define DBG_TAG "usb_pcdc"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define LED_PIN_0    BSP_IO_PORT_00_PIN_12 /* Onboard LED pins */

void hal_entry(void)
{
#if BSP_CFG_DCACHE_ENABLED
    SCB_DisableDCache();
    __DSB();
    __ISB();
    rt_kprintf("[SYS] DCache disabled at runtime.\n");
#endif

    rt_kprintf("\nHello RT-Thread!\n");
    rt_kprintf("==================================================\n");
    rt_kprintf("This example project is an driver usb_pcdc routine!\n");
    rt_kprintf("Current USB config: FS device on USB_IP0. Please connect the board USB-DEV FS port to the PC.\n");
    rt_kprintf("==================================================\n");

    LOG_I("\nTips:");
    LOG_I("This firmware is currently generated for USB FS peripheral mode.");
    LOG_I("usb_basic Stack: USB Speed = Full Speed, USB Module Number = USB_IP0_port.");

    rt_thread_t usb = rt_thread_create("usb_pcdc", usb_pcdc_app, RT_NULL, 1024, 20, 10);
    if(usb != RT_NULL)
    {
        rt_thread_startup(usb);
    }

    while (1)
    {
        rt_pin_write(LED_PIN_0, PIN_HIGH);
        rt_thread_mdelay(1000);
        rt_pin_write(LED_PIN_0, PIN_LOW);
        rt_thread_mdelay(1000);
    }
}
