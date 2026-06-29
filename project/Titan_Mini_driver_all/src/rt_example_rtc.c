#include <rtthread.h>
#include <rtdevice.h>
#include <time.h>
#ifdef RT_USING_ALARM
#include <drivers/alarm.h>
#endif
#include "rt_example.h"

#define RTC_NAME    "rtc"

#ifdef RT_USING_ALARM
static void user_alarm_callback(rt_alarm_t alarm, time_t timestamp)
{
    RT_UNUSED(alarm);
    RT_UNUSED(timestamp);

    rt_kprintf("user alarm callback function.\n");
}

void rtc_sample(void)
{
    rt_err_t ret = RT_EOK;
    time_t now = 0;
    rt_device_t device = RT_NULL;

    device = rt_device_find(RTC_NAME);
    if (device == RT_NULL)
    {
        rt_kprintf("find %s failed!\n", RTC_NAME);
        return;
    }

    if (rt_device_open(device, 0) != RT_EOK)
    {
        rt_kprintf("open %s failed!\n", RTC_NAME);
        return;
    }

    ret = set_date(2025, 8, 1);
    rt_kprintf("set RTC date to 2025-8-1\n");
    if (ret != RT_EOK)
    {
        rt_kprintf("set RTC date failed\n");
        return;
    }

    ret = set_time(15, 0, 0);
    if (ret != RT_EOK)
    {
        rt_kprintf("set RTC time failed\n");
        return;
    }

    rt_thread_mdelay(3000);
    get_timestamp(&now);
    rt_kprintf("now: %.*s", 25, ctime(&now));
}
MSH_CMD_EXPORT(rtc_sample, rtc sample);

void alarm_sample(void)
{
    struct rt_alarm_setup setup;
    static rt_alarm_t alarm = RT_NULL;
    time_t now = 0;
    struct tm p_tm;

    if (alarm != RT_NULL)
    {
        rt_kprintf("alarm already created\n");
        return;
    }

    now = get_timestamp(RT_NULL) + 1;
    gmtime_r(&now, &p_tm);

    setup.flag = RT_ALARM_SECOND;
    setup.wktime.tm_year = p_tm.tm_year;
    setup.wktime.tm_mon = p_tm.tm_mon;
    setup.wktime.tm_mday = p_tm.tm_mday;
    setup.wktime.tm_wday = p_tm.tm_wday;
    setup.wktime.tm_hour = p_tm.tm_hour;
    setup.wktime.tm_min = p_tm.tm_min;
    setup.wktime.tm_sec = p_tm.tm_sec;

    alarm = rt_alarm_create(user_alarm_callback, &setup);
    if (alarm == RT_NULL)
    {
        rt_kprintf("create alarm failed\n");
        return;
    }

    rt_alarm_start(alarm);
}
#else
void alarm_sample(void)
{
    rt_kprintf("alarm sample is unavailable: RT_USING_ALARM is not enabled.\n");
}
#endif
MSH_CMD_EXPORT(alarm_sample, alarm sample);
