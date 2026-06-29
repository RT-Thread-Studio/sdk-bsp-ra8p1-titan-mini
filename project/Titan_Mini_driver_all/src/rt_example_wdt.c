#include <rtthread.h>
#include <rtdevice.h>
#include "rt_example.h"

#define WDT_DEVICE_NAME     "wdt"
#define WDT_FEED_INTERVAL   1000

static rt_device_t g_wdt_dev = RT_NULL;
static rt_thread_t g_feed_thread = RT_NULL;

static void feed_dog_entry(void *parameter)
{
    int count = 0;

    RT_UNUSED(parameter);

    while (1)
    {
        if (count < 10)
        {
            rt_device_control(g_wdt_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, RT_NULL);
            rt_kprintf("[FeedDog] Feeding watchdog... %d\n", count);
        }
        else
        {
            rt_kprintf("[FeedDog] Simulate exception! Stop feeding.\n");
        }

        count++;
        rt_thread_mdelay(WDT_FEED_INTERVAL);
    }
}

int wdt_sample(void)
{
    g_wdt_dev = rt_device_find(WDT_DEVICE_NAME);
    if (g_wdt_dev == RT_NULL)
    {
        rt_kprintf("cannot find %s device!\n", WDT_DEVICE_NAME);
        return -RT_ERROR;
    }

    if (rt_device_control(g_wdt_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL) != RT_EOK)
    {
        rt_kprintf("start watchdog failed!\n");
        return -RT_ERROR;
    }

    g_feed_thread = rt_thread_create("feed_dog", feed_dog_entry, RT_NULL, 1024, 10, 10);
    if (g_feed_thread == RT_NULL)
    {
        rt_kprintf("create watchdog thread failed!\n");
        return -RT_ERROR;
    }

    rt_thread_startup(g_feed_thread);
    rt_kprintf("watchdog sample started\n");
    return RT_EOK;
}
MSH_CMD_EXPORT(wdt_sample, wdt_sample);
