/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include "rpmsg_comm.h"

#define DBG_TAG "hal.rpmsg"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static void rpmsg_hello(void)
{
    const char *hello = "hello";

    if (!rpmsg_comm_is_ready())
    {
        rt_kprintf("RPMSG is not ready\n");
        return;
    }

    if (rpmsg_comm_send(&rpmsg_comm, (void *)hello, 6) == RT_EOK)
    {
        rt_kprintf("M85 TX: %s\n", hello);
    }
    else
    {
        rt_kprintf("M85 TX failed\n");
    }
}
MSH_CMD_EXPORT(rpmsg_hello, send hello to M33);

void hal_entry(void)
{
    char buf[64];
    rt_uint32_t len;

    LOG_I("RPMSG master init");
    if (rpmsg_comm_master_init(&rpmsg_comm) != RT_EOK)
    {
        LOG_E("Failed to initialize RPMSG master");
        return;
    }
    LOG_I("RPMSG master ready");

    while (1)
    {
        if (rpmsg_comm_recv(&rpmsg_comm, buf, sizeof(buf), &len) == RT_EOK)
        {
            rt_kprintf("M85 RX: %.*s\n", (int)len, buf);
        }

        rt_thread_mdelay(100);
    }
}