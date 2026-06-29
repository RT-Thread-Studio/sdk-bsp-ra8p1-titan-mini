#include <rtthread.h>
#include <rtdevice.h>
#include <stdlib.h>
#include "rt_example.h"

#define PWM_DEV_NAME        "pwm6"
#define PWM_DEV_CHANNEL     0

static struct rt_device_pwm *g_pwm_dev = RT_NULL;

static int pwm_sample(int argc, char *argv[])
{
    rt_uint32_t period = 0;
    rt_uint32_t pulse = 0;

    if (argc != 3)
    {
        rt_kprintf("Usage: pwm_sample <period> <pulse>\n");
        rt_kprintf("Example: pwm_sample 500000 250000\n");
        return -RT_ERROR;
    }

    period = (rt_uint32_t) atoi(argv[1]);
    pulse = (rt_uint32_t) atoi(argv[2]);
    if ((period == 0) || (pulse > period))
    {
        rt_kprintf("invalid parameters, ensure period > 0 and pulse <= period.\n");
        return -RT_ERROR;
    }

    g_pwm_dev = (struct rt_device_pwm *) rt_device_find(PWM_DEV_NAME);
    if (g_pwm_dev == RT_NULL)
    {
        rt_kprintf("cannot find pwm device %s\n", PWM_DEV_NAME);
        return -RT_ERROR;
    }

    if (rt_pwm_set(g_pwm_dev, PWM_DEV_CHANNEL, period, pulse) != RT_EOK)
    {
        rt_kprintf("set pwm failed\n");
        return -RT_ERROR;
    }

    if (rt_pwm_enable(g_pwm_dev, PWM_DEV_CHANNEL) != RT_EOK)
    {
        rt_kprintf("enable pwm failed\n");
        return -RT_ERROR;
    }

    rt_kprintf("pwm started on %s channel %d, period=%u pulse=%u\n",
               PWM_DEV_NAME,
               PWM_DEV_CHANNEL,
               period,
               pulse);
    return RT_EOK;
}
MSH_CMD_EXPORT(pwm_sample, configure and start pwm output: pwm_sample <period> <pulse>);
