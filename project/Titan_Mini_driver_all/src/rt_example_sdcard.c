#include <rtthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <dfs_fs.h>
#include "rt_example.h"

void sdcard_sample(void)
{
    FILE *fp = RT_NULL;
    const char *test_file = "/sdcard/test_sdcard.txt";
    const char *test_data = "SD Card test data - Titan Mini Board";
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
        rt_kprintf("sdcard data mismatch!\n");
        rt_kprintf("written: %s\n", test_data);
        rt_kprintf("read: %s\n", read_buf);
        return;
    }

    unlink(test_file);
    rt_kprintf("sdcard sample passed\n");
}
MSH_CMD_EXPORT(sdcard_sample, sdcard file read write sample);
