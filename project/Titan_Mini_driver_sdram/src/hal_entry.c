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

#define SDRAM_TEST_WORDS   (8 * 1024 * 1024)   // 32MB / 4 bytes = 8M words
#define SDRAM_BASE_ADDR    (0x68000000)

static volatile uint32_t *sdram_buf = (uint32_t *)SDRAM_BASE_ADDR;
static uint32_t src_buf[SDRAM_TEST_WORDS] __attribute__((section(".sdram_noinit")));

static void dwt_init(void)
{
    /* Enable DWT */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t dwt_get_cycle(void)
{
    return DWT->CYCCNT;
}

void sdram_speed_test(void)
{
    uint32_t i;
    uint32_t count = 0;
    uint32_t start, end;
    uint32_t cpu_hz = SystemCoreClock;
    uint64_t cycles;
    double seconds;
    double mbps;

    rt_kprintf("\n========== SDRAM 32-BIT SPEED TEST ==========\n");
    rt_kprintf("CPU Freq: %u Hz\n", cpu_hz);
    rt_kprintf("Test Size: %u MB\n", (SDRAM_TEST_WORDS * 4) / (1024 * 1024));

    dwt_init();

    /* ================= 写测试 ================= */
    start = dwt_get_cycle();

    for (i = 0; i < SDRAM_TEST_WORDS; i++)
    {
        sdram_buf[i] = (uint32_t)(i ^ 0xA5A5A5A5);
    }

    end = dwt_get_cycle();

    cycles = (uint64_t)(end - start);
    seconds = (double)cycles / cpu_hz;
    mbps = ((double)SDRAM_TEST_WORDS * 4 / (1024.0 * 1024.0)) / seconds;

    rt_kprintf("Write:  %.2f MB/s  (%.3f ms)\n", mbps, seconds * 1000);

    /* ================= 读测试 ================= */
    volatile uint32_t temp;
    start = dwt_get_cycle();

    for (i = 0; i < SDRAM_TEST_WORDS; i++)
    {
        temp = sdram_buf[i];

        uint16_t expect_low16 = (uint16_t)((i ^ 0xA5A5A5A5) & 0xFFFF);
        uint16_t actual_low16 = (uint16_t)(temp & 0xFFFF);

        if (actual_low16 != expect_low16)
        {
            rt_kprintf("error: %d, expect_low16: %#x, actual_low16: %#x\n",
                       count,
                       expect_low16,
                       actual_low16);
            count++;
        }
    }

    end = dwt_get_cycle();

    cycles = (uint64_t)(end - start);
    seconds = (double)cycles / cpu_hz;
    mbps = ((double)SDRAM_TEST_WORDS * 4 / (1024.0 * 1024.0)) / seconds;

    rt_kprintf("Read:   %.2f MB/s  (%.3f ms)\n", mbps, seconds * 1000);

    /* ================= memcpy 测试 ================= */
    start = dwt_get_cycle();

    memcpy((void *)sdram_buf, src_buf, SDRAM_TEST_WORDS * 4);

    end = dwt_get_cycle();

    cycles = (uint64_t)(end - start);
    seconds = (double)cycles / cpu_hz;
    mbps = ((double)SDRAM_TEST_WORDS * 4 / (1024.0 * 1024.0)) / seconds;

    rt_kprintf("memcpy: %.2f MB/s  (%.3f ms)\n", mbps, seconds * 1000);

    rt_kprintf("===========================================\n\n");
}
MSH_CMD_EXPORT(sdram_speed_test, sdram_speed_test);
