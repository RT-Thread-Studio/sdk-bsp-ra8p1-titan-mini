#include <rtthread.h>
#include <board.h>
#include "hal_data.h"

#define DBG_TAG "sdram"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define EXT_SDRAM_ADDR ((uint32_t)0x68000000)
#define EXT_SDRAM_SIZE (0x2000000)

#define TEST_ADDRESS   0
#define TEST_BUF_SIZE  256

/* ============================================================ */
/* Fill + Check (32bit, only check low16) */
static void FillBuff32(uint32_t pattern)
{
    uint32_t i;
    uint32_t *pBuf;
    uint32_t err_cnt = 0;

    pBuf = (uint32_t *)EXT_SDRAM_ADDR;

    /* 写满 SDRAM */
    for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
    {
        *pBuf++ = pattern;
    }

    /* 校验（只校验低16位） */
    pBuf = (uint32_t *)EXT_SDRAM_ADDR;

    for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
    {
        uint32_t val = *pBuf++;

        if ((val & 0xFFFF) != (pattern & 0xFFFF))
        {
            LOG_W("FillBuff32 error offset:0x%08x read_low16:0x%04x should:0x%04x",
                  i * 4,
                  (uint16_t)(val & 0xFFFF),
                  (uint16_t)(pattern & 0xFFFF));

            if (++err_cnt >= 5)
            {
                LOG_W("Too many errors, skip round.");
                break;
            }
        }
    }
}

/* ============================================================ */
/* 写速度测试 (32bit, only check low16) */
void SDRAM_WriteSpeedTest32(void)
{
    uint32_t start, end;
    uint32_t i;
    uint32_t *pBuf;
    uint32_t duration;
    uint32_t val = 0;

    FillBuff32(0x55AA55AA);
    FillBuff32(0xAA55AA55);

    pBuf = (uint32_t *)EXT_SDRAM_ADDR;

    start = rt_tick_get_millisecond();

    for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
    {
        *pBuf++ = val++;
    }

    end = rt_tick_get_millisecond();
    duration = end - start;

    /* 校验（只校验低16位） */
    pBuf = (uint32_t *)EXT_SDRAM_ADDR;
    val = 0;

    for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
    {
        uint32_t read = *pBuf++;
        if ((read & 0xFFFF) != (val & 0xFFFF))
        {
            rt_kprintf("write check error at index=%d\r\n", i);
            break;
        }
        val++;
    }

    rt_kprintf("SDRAM write duration:%dms, speed:%dMB/s\r\n",
               duration,
               (EXT_SDRAM_SIZE / 1024 / 1024 * 1000) / duration);
}
MSH_CMD_EXPORT_ALIAS(SDRAM_WriteSpeedTest32, sdram_write, "SDRAM Write Test 32bit");

/* ============================================================ */
/* 读速度测试 (32bit) */
void SDRAM_ReadSpeedTest32(void)
{
    uint32_t start, end;
    uint32_t i;
    uint32_t *pBuf;
    volatile uint32_t temp;
    uint32_t duration;

    pBuf = (uint32_t *)EXT_SDRAM_ADDR;

    start = rt_tick_get_millisecond();

    for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
    {
        temp = *pBuf++;
    }

    end = rt_tick_get_millisecond();
    duration = end - start;

    rt_kprintf("SDRAM read duration:%dms, speed:%dMB/s\r\n",
               duration,
               (EXT_SDRAM_SIZE / 1024 / 1024 * 1000) / duration);
}
MSH_CMD_EXPORT_ALIAS(SDRAM_ReadSpeedTest32, sdram_read, "SDRAM Read Test 32bit");

/* ============================================================ */
/* 小块 32bit 测试（只打印低16位） */
static void SDRAM_ReadWriteTest32(void)
{
    uint32_t i;
    uint32_t *pBuf;

    pBuf = (uint32_t *)(EXT_SDRAM_ADDR + TEST_ADDRESS);

    for (i = 0; i < TEST_BUF_SIZE; i++)
    {
        pBuf[i] = 0xA55AA55A;
    }

    rt_kprintf("addr:0x%08X size:%d bytes\r\n",
               EXT_SDRAM_ADDR + TEST_ADDRESS,
               TEST_BUF_SIZE * 4);

    for (i = 0; i < TEST_BUF_SIZE; i++)
    {
        rt_kprintf(" %04X", (uint16_t)(pBuf[i] & 0xFFFF));

        if ((i & 7) == 7)
            rt_kprintf("\r\n");
    }
}
MSH_CMD_EXPORT_ALIAS(SDRAM_ReadWriteTest32, sdram_rw, "SDRAM RW Test 32bit");

/* ============================================================ */
/* Burn 测试 */
static void SDRAM_BurnTest32(void)
{
    for(int i=0;i<5;i++)
    {
        SDRAM_WriteSpeedTest32();
        SDRAM_ReadSpeedTest32();
        rt_thread_mdelay(5);
    }
}
MSH_CMD_EXPORT_ALIAS(SDRAM_BurnTest32, sdram_burn, "SDRAM Burn Test 32bit");
