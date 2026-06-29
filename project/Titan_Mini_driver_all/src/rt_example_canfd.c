#include <rtthread.h>
#include <rtdevice.h>
#include "rt_example.h"

#define CAN_DEV_NAME    "canfd0"

static struct rt_semaphore g_can_rx_sem;
static rt_device_t g_can_dev = RT_NULL;
static rt_bool_t g_can_started = RT_FALSE;

static rt_err_t can_rx_indicate(rt_device_t dev, rt_size_t size)
{
    RT_UNUSED(dev);
    RT_UNUSED(size);

    rt_sem_release(&g_can_rx_sem);

    return RT_EOK;
}

static void can_rx_thread(void *parameter)
{
    struct rt_can_msg rxmsg = {0};

    RT_UNUSED(parameter);

    rt_device_set_rx_indicate(g_can_dev, can_rx_indicate);

    while (1)
    {
        rxmsg.hdr_index = -1;
        rt_sem_take(&g_can_rx_sem, RT_WAITING_FOREVER);

        if (rt_device_read(g_can_dev, 0, &rxmsg, sizeof(rxmsg)) <= 0)
        {
            rt_kprintf("canfd read failed!\n");
            continue;
        }

        rt_kprintf("ID:%x message:", rxmsg.id);
        for (rt_uint8_t i = 0; i < rxmsg.len; i++)
        {
            rt_kprintf("%d ", rxmsg.data[i]);
        }
        rt_kprintf("\n");
    }
}

static void can_tx_thread(void *parameter)
{
    struct rt_can_msg txmsg = {0};

    RT_UNUSED(parameter);

    txmsg.id = 0x78;
    txmsg.ide = RT_CAN_STDID;
    txmsg.rtr = RT_CAN_DTR;
    txmsg.len = 8;

    while (1)
    {
        for (rt_uint8_t i = 0; i < txmsg.len; i++)
        {
            txmsg.data[i] = i;
        }

        if (rt_device_write(g_can_dev, 0, &txmsg, sizeof(txmsg)) > 0)
        {
            rt_kprintf("canfd tx ok\n");
        }
        else
        {
            rt_kprintf("canfd tx failed\n");
        }

        rt_thread_mdelay(1000);
    }
}

int canfd_test(void)
{
    rt_thread_t rx_thread = RT_NULL;
    rt_thread_t tx_thread = RT_NULL;

    if (g_can_started)
    {
        rt_kprintf("canfd demo already started\n");
        return RT_EOK;
    }

    rt_sem_init(&g_can_rx_sem, "canrx", 0, RT_IPC_FLAG_FIFO);

    g_can_dev = rt_device_find(CAN_DEV_NAME);
    if (g_can_dev == RT_NULL)
    {
        rt_kprintf("can't find %s\n", CAN_DEV_NAME);
        return -RT_ERROR;
    }

    if (rt_device_open(g_can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("open %s failed\n", CAN_DEV_NAME);
        return -RT_ERROR;
    }

    rx_thread = rt_thread_create("can_rx", can_rx_thread, RT_NULL, 2048, 15, 10);
    tx_thread = rt_thread_create("can_tx", can_tx_thread, RT_NULL, 2048, 14, 10);
    if ((rx_thread == RT_NULL) || (tx_thread == RT_NULL))
    {
        rt_kprintf("create canfd thread failed\n");
        return -RT_ERROR;
    }

    rt_thread_startup(rx_thread);
    rt_thread_startup(tx_thread);
    g_can_started = RT_TRUE;

    rt_kprintf("canfd test started\n");
    return RT_EOK;
}
MSH_CMD_EXPORT(canfd_test, canfd test);
