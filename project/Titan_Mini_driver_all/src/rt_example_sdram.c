#include <rtthread.h>
#include <string.h>
#include "hal_data.h"
#include "rt_example.h"

#define SDRAM_TEST_WORDS   (8 * 1024 * 1024)
#define SDRAM_BASE_ADDR    (0x68000000)

static volatile uint32_t *sdram_buf = (uint32_t *) SDRAM_BASE_ADDR;
static uint32_t src_buf[SDRAM_TEST_WORDS] __attribute__((section(".sdram_noinit")));

static void dwt_init(void)
{
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
    uint32_t i = 0;
    uint32_t count = 0;
    uint32_t start = 0;
    uint32_t end = 0;
    uint32_t cpu_hz = SystemCoreClock;
    uint64_t cycles = 0;
    double seconds = 0;
    double mbps = 0;
    volatile uint32_t temp = 0;

    rt_kprintf("\n========== SDRAM 32-BIT SPEED TEST ==========\n");
    rt_kprintf("CPU Freq: %u Hz\n", cpu_hz);
    rt_kprintf("Test Size: %u MB\n", (SDRAM_TEST_WORDS * 4) / (1024 * 1024));

    dwt_init();

    start = dwt_get_cycle();
    for (i = 0; i < SDRAM_TEST_WORDS; i++)
    {
        sdram_buf[i] = (uint32_t) (i ^ 0xA5A5A5A5);
    }
    end = dwt_get_cycle();

    cycles = (uint64_t) (end - start);
    seconds = (double) cycles / cpu_hz;
    mbps = ((double) SDRAM_TEST_WORDS * 4 / (1024.0 * 1024.0)) / seconds;
    rt_kprintf("Write:  %.2f MB/s  (%.3f ms)\n", mbps, seconds * 1000);

    start = dwt_get_cycle();
    for (i = 0; i < SDRAM_TEST_WORDS; i++)
    {
        uint16_t expect_low16;
        uint16_t actual_low16;

        temp = sdram_buf[i];
        expect_low16 = (uint16_t) ((i ^ 0xA5A5A5A5) & 0xFFFF);
        actual_low16 = (uint16_t) (temp & 0xFFFF);
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

    cycles = (uint64_t) (end - start);
    seconds = (double) cycles / cpu_hz;
    mbps = ((double) SDRAM_TEST_WORDS * 4 / (1024.0 * 1024.0)) / seconds;
    rt_kprintf("Read:   %.2f MB/s  (%.3f ms)\n", mbps, seconds * 1000);

    start = dwt_get_cycle();
    memcpy((void *) sdram_buf, src_buf, SDRAM_TEST_WORDS * 4);
    end = dwt_get_cycle();

    cycles = (uint64_t) (end - start);
    seconds = (double) cycles / cpu_hz;
    mbps = ((double) SDRAM_TEST_WORDS * 4 / (1024.0 * 1024.0)) / seconds;
    rt_kprintf("memcpy: %.2f MB/s  (%.3f ms)\n", mbps, seconds * 1000);

    rt_kprintf("===========================================\n\n");
}
MSH_CMD_EXPORT(sdram_speed_test, sdram_speed_test);
