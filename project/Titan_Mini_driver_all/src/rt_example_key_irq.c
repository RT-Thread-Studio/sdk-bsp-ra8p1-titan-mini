#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "rt_example.h"

#define LED_PIN_G   BSP_IO_PORT_01_PIN_08
#define KEY_PIN     BSP_IO_PORT_02_PIN_01

static volatile rt_bool_t g_key_flag = RT_FALSE;

static void key_irq_callback(void *args)
{
    RT_UNUSED(args);

    if (g_key_flag)
    {
        rt_pin_write(LED_PIN_G, PIN_HIGH);
    }
    else
    {
        rt_pin_write(LED_PIN_G, PIN_LOW);
    }

    g_key_flag = g_key_flag ? RT_FALSE : RT_TRUE;
}

void key_irq_sample(void)
{
    rt_pin_mode(LED_PIN_G, PIN_MODE_OUTPUT);
    rt_pin_mode(KEY_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(KEY_PIN, PIN_IRQ_MODE_FALLING, key_irq_callback, RT_NULL);
    rt_pin_irq_enable(KEY_PIN, PIN_IRQ_ENABLE);

    rt_kprintf("key irq sample started, press key to toggle green led.\n");
}
MSH_CMD_EXPORT(key_irq_sample, key interrupt sample);
