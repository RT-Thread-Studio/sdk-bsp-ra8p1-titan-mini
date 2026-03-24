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

#define TEXT_NUMBER_SIZE 1024
#define SPI_BUS_NAME    "spi1"
#define SPI_NAME        "spi10"

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

void spi_loop_test(void)
{
    static uint8_t sendbuf[TEXT_NUMBER_SIZE] = {0};
    static uint8_t readbuf[TEXT_NUMBER_SIZE] = {0};

    for (int i = 0; i < sizeof(readbuf); i++)
    {
        sendbuf[i] = i;
    }

    static struct rt_spi_device *spi_dev = RT_NULL;
    struct rt_spi_configuration cfg;

    rt_hw_spi_device_attach(SPI_BUS_NAME, SPI_NAME, NULL);

    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB | RT_SPI_NO_CS;
    cfg.max_hz = 1 * 1000 * 1000;

    spi_dev = (struct rt_spi_device *)rt_device_find(SPI_NAME);
    if (RT_NULL == spi_dev)
    {
        rt_kprintf("spi sample run failed! can't find %s device!\n", SPI_NAME);
        return;
    }
    rt_spi_configure(spi_dev, &cfg);

    rt_kprintf("%s send:\n", SPI_NAME);
    for (int i = 0; i < sizeof(sendbuf); i++)
    {
        rt_kprintf("%02x ", sendbuf[i]);
    }

    rt_spi_transfer(spi_dev, sendbuf, readbuf, sizeof(sendbuf));
    rt_kprintf("\n\n%s rcv:\n", SPI_NAME);

    for (int i = 0; i < sizeof(readbuf); i++)
    {
        if (readbuf[i] != sendbuf[i])
        {
            rt_kprintf("SPI test fail!!!\n");
            break;
        }
        else
            rt_kprintf("%02x ", readbuf[i]);
    }

    rt_kprintf("\n\n");
    rt_kprintf("SPI test end\n");
}
MSH_CMD_EXPORT(spi_loop_test, test spi1);
