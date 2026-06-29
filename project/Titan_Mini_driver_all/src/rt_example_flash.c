#include <rtthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <dfs_fs.h>
#include "hal_data.h"
#include "rt_example.h"

#define FLASH_SPEED_TEST_SIZE   (64 * 1024)          /* 64KB 测试文件 */
#define FLASH_SPEED_BUF_SIZE    4096                 /* 单次读写块大小 */

static void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

static rt_uint32_t dwt_get_cycle(void)
{
    return DWT->CYCCNT;
}

void flash_sample(void)
{
    FILE *fp = RT_NULL;
    const char *test_file = "/fal/test_flash.txt";
    const char *test_data = "Flash test data - Titan Mini Board";
    char read_buf[64] = {0};

    fp = fopen(test_file, "w");
    if (fp == RT_NULL)
    {
        rt_kprintf("failed to create test file: %s\n", test_file);
        return;
    }

    if (fputs(test_data, fp) < 0)
    {
        rt_kprintf("failed to write test data\n");
        fclose(fp);
        return;
    }
    fclose(fp);

    fp = fopen(test_file, "r");
    if (fp == RT_NULL)
    {
        rt_kprintf("failed to open test file for reading\n");
        return;
    }

    if (fgets(read_buf, sizeof(read_buf), fp) == RT_NULL)
    {
        rt_kprintf("failed to read test data\n");
        fclose(fp);
        return;
    }
    fclose(fp);

    if (strcmp(read_buf, test_data) != 0)
    {
        rt_kprintf("flash data mismatch!\n");
        rt_kprintf("written: %s\n", test_data);
        rt_kprintf("read: %s\n", read_buf);
        return;
    }

    unlink(test_file);
    rt_kprintf("flash sample passed\n");
}
MSH_CMD_EXPORT(flash_sample, flash file read write sample);

/* 统计写入或读取带宽（KB/s 与 KB/ms 换算） */
static void flash_speed_report(const char *op, rt_uint32_t cycles, rt_uint32_t bytes)
{
    rt_uint32_t cpu_hz = SystemCoreClock;
    double seconds = (double)cycles / cpu_hz;
    double kbytes   = (double)bytes / 1024.0;
    double kbps     = (seconds > 0) ? (kbytes / seconds) : 0;

    rt_kprintf("%-6s: %.2f KB/s  (%.3f ms, %u bytes)\n",
               op, kbps, seconds * 1000.0, bytes);
}

void flash_speed_test(void)
{
    const char *test_file = "/fal/speed_test.bin";
    static rt_uint8_t wr_buf[FLASH_SPEED_BUF_SIZE];
    static rt_uint8_t rd_buf[FLASH_SPEED_BUF_SIZE];
    FILE *fp = RT_NULL;
    rt_uint32_t start = 0, end = 0;
    rt_uint32_t total = FLASH_SPEED_TEST_SIZE;
    rt_uint32_t offset = 0;
    rt_uint32_t i = 0;

    rt_kprintf("\n========== Flash Speed Test ==========\n");
    rt_kprintf("CPU Freq: %u Hz\n", SystemCoreClock);
    rt_kprintf("Test Size: %u KB (block %u B)\n", total / 1024, FLASH_SPEED_BUF_SIZE);

    /* 准备写入数据：伪随机模式 */
    for (i = 0; i < FLASH_SPEED_BUF_SIZE; i++)
    {
        wr_buf[i] = (rt_uint8_t)(i ^ 0x5A);
    }

    dwt_init();

    /* 1. 顺序写入测速 */
    fp = fopen(test_file, "wb");
    if (fp == RT_NULL)
    {
        rt_kprintf("failed to create test file: %s\n", test_file);
        return;
    }

    start = dwt_get_cycle();
    for (offset = 0; offset < total; offset += FLASH_SPEED_BUF_SIZE)
    {
        if (fwrite(wr_buf, 1, FLASH_SPEED_BUF_SIZE, fp) != FLASH_SPEED_BUF_SIZE)
        {
            rt_kprintf("write failed at offset %u\n", offset);
            fclose(fp);
            return;
        }
    }
    fflush(fp);
    fclose(fp);
    end = dwt_get_cycle();
    flash_speed_report("Write", end - start, total);

    /* 2. 顺序读取测速 + 数据校验 */
    fp = fopen(test_file, "rb");
    if (fp == RT_NULL)
    {
        rt_kprintf("failed to open test file for reading\n");
        return;
    }

    start = dwt_get_cycle();
    for (offset = 0; offset < total; offset += FLASH_SPEED_BUF_SIZE)
    {
        if (fread(rd_buf, 1, FLASH_SPEED_BUF_SIZE, fp) != FLASH_SPEED_BUF_SIZE)
        {
            rt_kprintf("read failed at offset %u\n", offset);
            fclose(fp);
            return;
        }
        if (memcmp(rd_buf, wr_buf, FLASH_SPEED_BUF_SIZE) != 0)
        {
            rt_kprintf("data mismatch at offset %u\n", offset);
            fclose(fp);
            return;
        }
    }
    end = dwt_get_cycle();
    flash_speed_report("Read", end - start, total);
    fclose(fp);

    /* 3. 清理测试文件 */
    start = dwt_get_cycle();
    if (unlink(test_file) != 0)
    {
        rt_kprintf("failed to delete test file\n");
        return;
    }
    end = dwt_get_cycle();
    flash_speed_report("Erase", end - start, total);

    rt_kprintf("======================================\n\n");
}
MSH_CMD_EXPORT(flash_speed_test, flash read/write speed test);
