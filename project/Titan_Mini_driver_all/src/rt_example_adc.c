#include <rtthread.h>
#include <rtdevice.h>
#include "rt_example.h"

#define ADC_DEV_NAME        "adc0"
#define ADC_DEV_CHANNEL     0
#define REFER_VOLTAGE       330
#define CONVERT_BITS        (1 << 16)
#define ADC_INVALID_VALUE   0xFFFF
#define ADC_WARN_THRESHOLD  (CONVERT_BITS * 95 / 100)

static void adc_vol_sample(void *parameter)
{
    rt_adc_device_t adc_dev = RT_NULL;
    rt_uint32_t value = 0;
    rt_uint32_t vol = 0;

    RT_UNUSED(parameter);

    adc_dev = (rt_adc_device_t) rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
        return;
    }

    if (rt_adc_enable(adc_dev, ADC_DEV_CHANNEL) != RT_EOK)
    {
        rt_kprintf("adc enable failed! channel = %d\n", ADC_DEV_CHANNEL);
        return;
    }

    rt_kprintf("adc sample started, input must be lower than Vref.\n");
    rt_kprintf("warning: applying 3.3V directly may overflow ADC.\n");

    while (1)
    {
        value = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);

        if (value == ADC_INVALID_VALUE)
        {
            rt_kprintf("adc overrange: input is too high, please keep it below Vref.\n");
        }
        else
        {
            rt_kprintf("the value is :%d\n", value);

            vol = value * REFER_VOLTAGE / CONVERT_BITS;
            if (value >= ADC_WARN_THRESHOLD)
            {
                rt_kprintf("warning: near full scale, the voltage is :%d.%02d\n",
                           vol / 100,
                           vol % 100);
            }
            else
            {
                rt_kprintf("the voltage is :%d.%02d\n", vol / 100, vol % 100);
            }
        }

        rt_thread_mdelay(1000);
    }
}

void adc_sample(void)
{
    rt_thread_t adc = rt_thread_create("adc",
                                       adc_vol_sample,
                                       RT_NULL,
                                       1024,
                                       10,
                                       10);

    if (adc == RT_NULL)
    {
        rt_kprintf("create adc thread failed!\n");
        return;
    }

    rt_thread_startup(adc);
}
MSH_CMD_EXPORT(adc_sample, adc sample demo);
