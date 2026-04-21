/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2024-03-11     kurisaw       first version
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>

#define DBG_TAG "canfd"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* 配置 LED 灯引脚 */
#define LED_PIN_R   BSP_IO_PORT_01_PIN_09

#define CAN_DEV_NAME       "canfd0"

static struct rt_semaphore rx_sem;     /* 用于接收消息的信号量 */
static rt_device_t can_dev = RT_NULL;

/* 接收数据回调函数 */
static rt_err_t can_rx_call(rt_device_t dev, rt_size_t size)
{
    /* CAN 接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

/* ===================== 接收线程 ===================== */
static void can_rx_thread(void *parameter)
{
    struct rt_can_msg rxmsg = {0};

    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(can_dev, can_rx_call);

    while (1)
    {
        /* hdr 值为 - 1，表示直接从 uselist 链表读取数据 */
        rxmsg.hdr_index = -1;
        /* 阻塞等待接收信号量 */
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        /* 从 CAN 读取一帧数据 */
        rt_device_read(can_dev, 0, &rxmsg, sizeof(rxmsg));
        /* 打印数据 ID 及内容 */
        rt_kprintf("ID:%x\tmessege:", rxmsg.id);
        for(int i = 0; i < 8; i++)
        {
            rt_kprintf("%d",  rxmsg.data[i]);
        }
        rt_kprintf("\n");
    }
}

/* ===================== 发送线程 ===================== */
static void can_tx_thread(void *parameter)
{
    struct rt_can_msg txmsg = {0};

    txmsg.id = 0x78;              /* ID 为 0x78 */
    txmsg.ide = RT_CAN_STDID;     /* 标准格式 */
    txmsg.rtr = RT_CAN_DTR;       /* 数据帧 */
    txmsg.len = 8;                /* 数据长度为 8 */
    /* 待发送的 8 字节数据 */
    rt_memset(txmsg.data,0,sizeof(txmsg.data));

    while (1)
    {
        for (int i = 0; i < 8; i++)
        {
            txmsg.data[i] = i;
        }

        if (rt_device_write(can_dev, 0, &txmsg, sizeof(txmsg)) > 0)
        {
            LOG_I("TX OK");
        }
        else
        {
            LOG_E("TX FAIL");
        }

        rt_thread_mdelay(1000);
    }
}

int canfd_test(void)
{
    /* 初始化 CAN 接收信号量 */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);

    can_dev = rt_device_find(CAN_DEV_NAME);
    if (!can_dev)
    {
        LOG_E("can't find %s", CAN_DEV_NAME);
        return -RT_ERROR;
    }

    if (rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        LOG_E("open device failed");
        return -RT_ERROR;
    }

    /* 创建接收线程 */
    rt_thread_t rx_thread = rt_thread_create("can_rx",
                                             can_rx_thread,
                                             RT_NULL,
                                             2048,
                                             15,
                                             10);
    if (rx_thread)
        rt_thread_startup(rx_thread);

    /* 创建发送线程 */
    rt_thread_t tx_thread = rt_thread_create("can_tx",
                                             can_tx_thread,
                                             RT_NULL,
                                             2048,
                                             14,
                                             10);
    if (tx_thread)
        rt_thread_startup(tx_thread);

    LOG_I("CAN FD test started");

    return RT_EOK;
}
MSH_CMD_EXPORT(canfd_test, canfd test);

void hal_entry(void)
{
    rt_kprintf("\nHello RT-Thread!\n");
    rt_kprintf("============================================================\n");
    rt_kprintf("This example project is a Titan Board Mini CAN routine!\n");
    rt_kprintf("============================================================\n");

    rt_pin_mode(LED_PIN_R, PIN_MODE_OUTPUT);

    while(1)
    {
        rt_pin_write(LED_PIN_R, PIN_LOW);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN_R, PIN_HIGH);
        rt_thread_mdelay(500);
    }
}
