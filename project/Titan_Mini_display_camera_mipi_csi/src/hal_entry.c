/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2024-03-11     kurisaW       first version
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>
#include "camera_layer.h"
#include "camera_layer_config.h"

#define LED_PIN     BSP_IO_PORT_00_PIN_12 /* Onboard LED pins */

#define DISPLAY_SCREEN_WIDTH              (800)
#define DISPLAY_SCREEN_HEIGHT             (480)

extern struct rt_completion ceu_completion;
uint8_t display_layer1_buff_select = 0;
static void i2c_scan(int argc, char **argv)
{
    struct rt_i2c_bus_device *i2c_bus;

    if (argc < 2)
    {
        rt_kprintf("Usage: i2c_scan <bus_name>\n");
        return;
    }

    i2c_bus = rt_i2c_bus_device_find(argv[1]);
    if (i2c_bus == RT_NULL) {
        rt_kprintf("I2C bus %s not found!\n", argv[1]);
        return;
    }

    rt_kprintf("Scanning I2C bus %s...\n", argv[1]);
    rt_kprintf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

    for (rt_uint8_t i = 0; i < 0x80; i++)
    {
        if (i % 16 == 0)
        {
            rt_kprintf("%02x: ", i);
        }

        struct rt_i2c_msg msgs;
        rt_uint8_t dummy;

        msgs.addr = i;
        msgs.flags = RT_I2C_RD;
        msgs.buf = &dummy;
        msgs.len = 1;

        if (rt_i2c_transfer(i2c_bus, &msgs, 1) == 1) {
            rt_kprintf("%02x ", i);
        } else {
            rt_kprintf("-- ");
        }

        if (i % 16 == 15) rt_kprintf("\n");
    }
}
MSH_CMD_EXPORT(i2c_scan, Scan I2C bus);
void hal_entry(void)
{
    rt_kprintf ("\nHello Titan Board!\n");
    rt_kprintf ("===========================================================\n");
    rt_kprintf ("This example project is an mipi-csi camera display routine!\n");
    rt_kprintf ("===========================================================\n");

    
    // Initialize camera interface
    fsp_err_t fsp_status = FSP_SUCCESS;
    fsp_status = camera_init(false);
    if(FSP_SUCCESS != fsp_status)
    {
        rt_kprintf ("camera_init fail!\n");
        return;
    }

    camera_image_buffer_initialize ();

    camera_capture_start ();

#if defined(VIN_CFG_USE_RUNTIME_BUFFER)
    rt_kprintf("The vin driver uses hardware mailboxes for the buffer.\n");
#else
    rt_kprintf("The vin driver uses isr for the buffer.\n");
#endif

    while (1)
    {
#ifndef VIN_CFG_USE_RUNTIME_BUFFER
        rt_completion_wait(&ceu_completion, RT_WAITING_FOREVER);
#endif

        /* Draw camera image to display buffer */
        uint16_t * p_src  = (uint16_t *)camera_data_ready_buffer_pointer_get();
        uint16_t * p_dest = (uint16_t *)&fb_background[display_layer1_buff_select][0];
        int x_offset = DISPLAY_SCREEN_WIDTH - CAMERA_CAPTURE_IMAGE_WIDTH;

        for(int y = 0; y < CAMERA_CAPTURE_IMAGE_HEIGHT; y++)
        {
            for(int x = 0; x < CAMERA_CAPTURE_IMAGE_WIDTH; x++)
            {
                *(p_dest + (y * DISPLAY_SCREEN_WIDTH) + (x_offset + x)) = *(p_src + (y * CAMERA_CAPTURE_IMAGE_WIDTH) + x);
            }
        }

        rt_thread_mdelay (5);
    }
}
