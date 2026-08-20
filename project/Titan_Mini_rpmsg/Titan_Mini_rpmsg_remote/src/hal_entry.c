/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <rtthread.h>
#include "rpmsg_comm.h"

void hal_entry(void)
{
    char buf[64];
    rt_uint32_t len;

    rt_kprintf("\n==================================================\n");
    rt_kprintf("This is M33 Core1!\n");
    rt_kprintf("Hello, Titan Board Mini!\n");
    rt_kprintf("==================================================\n");

    if (rpmsg_comm_remote_init(&rpmsg_comm) != RT_EOK)
    {
        rt_kprintf("Remote RPMSG init failed\n");
    }

    while (1)
    {
        if (rpmsg_comm_recv(&rpmsg_comm, buf, sizeof(buf), &len) == RT_EOK)
        {
            rt_kprintf("M33 RX: %.*s\n", (int)len, buf);

            const char *hello = "hello";
            if (rpmsg_comm_send(&rpmsg_comm, (void *)hello, 6) == RT_EOK)
            {
                rt_kprintf("M33 TX: %s\n", hello);
            }
            else
            {
                rt_kprintf("M33 TX failed\n");
            }
        }

        rt_thread_mdelay(10);
    }
}
