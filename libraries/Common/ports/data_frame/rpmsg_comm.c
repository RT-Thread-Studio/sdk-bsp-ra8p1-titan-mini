/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2025-06-24     kurisaW       first version
 */

#include "rpmsg_comm.h"
#include <stdlib.h>

#define DBG_TAG "rpmsg.comm"
 #define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define RPMSG_SEND_TIMEOUT_MS 100U
#define RPMSG_LINK_WAIT_TIMEOUT_MS 20U

static void nameservice_isr_cb(rt_uint32_t new_ept, const char *new_ept_name, rt_uint32_t flags, void *user_data)
{
    rpmsg_comm_t *comm = (rpmsg_comm_t *)user_data;

    if (flags == (rt_uint32_t)RL_NS_CREATE)
    {
        comm->remote_addr = new_ept;
        comm->ready = RT_TRUE;
    }
    else if (flags == (rt_uint32_t)RL_NS_DESTROY)
    {
        comm->ready = RT_FALSE;
    }

    RT_UNUSED(new_ept_name);
}

rt_err_t rpmsg_comm_master_init(rpmsg_comm_t *comm)
{
    RT_ASSERT(comm != RT_NULL);

    comm->rpmsg_inst = RT_NULL;
    comm->queue = RT_NULL;
    comm->ept = RT_NULL;
    comm->tx_mb = RT_NULL;
    comm->ns_handle = RT_NULL;
    comm->remote_addr = RPMSG_LOCAL_EPT_ADDR;
    comm->ready = RT_FALSE;

    comm->rpmsg_inst = rpmsg_lite_master_init((void *)RPMSG_SHMEM_BASE, RPMSG_SHMEM_SIZE,
                                              RL_PLATFORM_RA8P1_M85_LINK_ID, RL_NO_FLAGS);
    if (!comm->rpmsg_inst)
    {
        LOG_E("rpmsg_lite_master_init failed!");
        return RT_ERROR;
    }

    (void)rpmsg_lite_wait_for_link_up(comm->rpmsg_inst, RPMSG_LINK_WAIT_TIMEOUT_MS);

    comm->queue = rpmsg_queue_create(comm->rpmsg_inst);
    comm->ept = rpmsg_lite_create_ept(comm->rpmsg_inst, RPMSG_LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, comm->queue);
    comm->ns_handle = rpmsg_ns_bind(comm->rpmsg_inst, nameservice_isr_cb, comm);
    if (!comm->ept || !comm->queue || !comm->ns_handle)
    {
        LOG_E("Failed to create endpoint or queue!");
        rpmsg_comm_deinit(comm);
        return RT_ERROR;
    }

    comm->tx_mb = rt_mb_create("rpmsg_tx", RPMSG_TX_MB_SIZE, RT_IPC_FLAG_FIFO);
    if (!comm->tx_mb)
    {
        LOG_E("Failed to create tx mailbox!");
        rpmsg_comm_deinit(comm);
        return RT_ERROR;
    }

    comm->ready = RT_TRUE;
    rt_thread_mdelay(100);
    LOG_I("Master rpmsg communication initialized\n");
    return RT_EOK;
}

rt_err_t rpmsg_comm_remote_init(rpmsg_comm_t *comm)
{
    RT_ASSERT(comm != RT_NULL);

    comm->rpmsg_inst = RT_NULL;
    comm->queue = RT_NULL;
    comm->ept = RT_NULL;
    comm->tx_mb = RT_NULL;
    comm->ns_handle = RT_NULL;
    comm->remote_addr = RPMSG_LOCAL_EPT_ADDR;
    comm->ready = RT_FALSE;

    comm->rpmsg_inst = rpmsg_lite_remote_init((void *)RPMSG_SHMEM_BASE, RL_PLATFORM_RA8P1_M33_LINK_ID, RL_NO_FLAGS);
    if (!comm->rpmsg_inst)
    {
        LOG_E("rpmsg_lite_remote_init failed!");
        return RT_ERROR;
    }

    rpmsg_lite_wait_for_link_up(comm->rpmsg_inst, RL_BLOCK);
    LOG_I("Link is up!");

    comm->queue = rpmsg_queue_create(comm->rpmsg_inst);
    comm->ept = rpmsg_lite_create_ept(comm->rpmsg_inst, RPMSG_LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, comm->queue);
    if (!comm->ept || !comm->queue)
    {
        LOG_E("Failed to create endpoint, queue, or nameservice!");
        rpmsg_comm_deinit(comm);
        return RT_ERROR;
    }

    if (rpmsg_ns_announce(comm->rpmsg_inst, comm->ept,
                          RPMSG_SERVICE_NAME, (uint32_t)RL_NS_CREATE) != RL_SUCCESS)
    {
        LOG_E("Failed to announce RPMSG endpoint");
        rpmsg_comm_deinit(comm);
        return RT_ERROR;
    }
    comm->ready = RT_TRUE;

    comm->tx_mb = rt_mb_create("rpmsg_tx", RPMSG_TX_MB_SIZE, RT_IPC_FLAG_FIFO);
    if (!comm->tx_mb)
    {
        LOG_E("Failed to create tx mailbox!");
        rpmsg_comm_deinit(comm);
        return RT_ERROR;
    }

    rt_thread_mdelay(100);
    LOG_I("Remote rpmsg communication initialized\n");
    return RT_EOK;
}

void rpmsg_comm_deinit(rpmsg_comm_t *comm)
{
    if (!comm)
    {
        return;
    }

    if (comm->ept)
    {
        rpmsg_lite_destroy_ept(comm->rpmsg_inst, comm->ept);
        comm->ept = RT_NULL;
    }
    if (comm->queue)
    {
        rpmsg_queue_destroy(comm->rpmsg_inst, comm->queue);
        comm->queue = RT_NULL;
    }
    if (comm->ns_handle)
    {
        rpmsg_ns_unbind(comm->rpmsg_inst, comm->ns_handle);
        comm->ns_handle = RT_NULL;
    }
    if (comm->rpmsg_inst)
    {
        rpmsg_lite_deinit(comm->rpmsg_inst);
        comm->rpmsg_inst = RT_NULL;
    }
    if (comm->tx_mb)
    {
        rt_mb_delete(comm->tx_mb);
        comm->tx_mb = RT_NULL;
    }
    LOG_I("Rpmsg communication deinitialized");
}

rt_err_t rpmsg_comm_send(rpmsg_comm_t *comm, void *msg, rt_size_t size)
{
    RT_ASSERT(comm != RT_NULL && msg != RT_NULL);

    if (!comm->rpmsg_inst || !comm->ept || !comm->ready || (size == 0U))
    {
        LOG_W("RPMSG send requested before endpoint is ready");
        return RT_ERROR;
    }

    if (rpmsg_lite_send(comm->rpmsg_inst, comm->ept, comm->remote_addr,
                        (char *)msg, size, RPMSG_SEND_TIMEOUT_MS) != RL_SUCCESS)
    {
        LOG_E("Failed to send message!");
        return RT_ERROR;
    }
    return RT_EOK;
}

rt_err_t rpmsg_comm_recv(rpmsg_comm_t *comm, void *msg, rt_size_t size, rt_uint32_t *len)
{
    RT_ASSERT(comm != RT_NULL && msg != RT_NULL && len != RT_NULL);

    if (rpmsg_queue_recv(comm->rpmsg_inst, comm->queue, (rt_uint32_t *)&comm->remote_addr,
                         (char *)msg, size, len, RL_DONT_BLOCK) == RL_SUCCESS)
    {
        comm->ready = RT_TRUE;
        return RT_EOK;
    }
    return RT_ERROR;
}

rt_bool_t rpmsg_comm_is_ready(void)
{
    return rpmsg_comm.ready;
}

rt_err_t rpmsg_send_to_core0(int argc, char *argv[])
{
    if (argc != 5)
    {
        rt_kprintf("Usage: rpmsg_send_to_core0 <index> <left_rear_rpm> <right_rear_rpm> <front_steering_angle>\n");
        rt_kprintf("Example: rpmsg_send_to_core0 1 1500.0 1520.5 12.3\n");
        return -RT_ERROR;
    }

    for (int i = 1; i < argc; i++)
    {
        if (argv[i] == NULL || strlen(argv[i]) == 0)
        {
            rt_kprintf("Error: Argument %d is empty\n", i);
            return -RT_ERROR;
        }
    }

    unsigned long index_value = 0;
    rt_uint32_t index = 0;
    float left_rear_vel = 0.0f;
    float right_rear_vel = 0.0f;
    float front_steering_angle = 0.0f;

    if (sscanf(argv[1], "%lu", &index_value) != 1)
    {
        rt_kprintf("Error: Invalid index format\n");
        return -RT_ERROR;
    }
    index = (rt_uint32_t)index_value;

    left_rear_vel = (float) atof(argv[2]);
    right_rear_vel = (float) atof(argv[3]);
    front_steering_angle = (float) atof(argv[4]);

    m33_to_m85_msg_t *tx_msg = (m33_to_m85_msg_t *)rt_malloc(sizeof(m33_to_m85_msg_t));
    if (!tx_msg)
    {
        LOG_E("Failed to allocate memory for tx message!");
        return -RT_ENOMEM;
    }
    rt_memset(tx_msg, 0, sizeof(m33_to_m85_msg_t));
    tx_msg->type = RPMSG_M33_TO_M85_TYPE_CONTROL;

    tx_msg->index = index;
    tx_msg->left_rear_vel = left_rear_vel;
    tx_msg->right_rear_vel = right_rear_vel;
    tx_msg->front_steering_angle = front_steering_angle;
    tx_msg->checksum = rpmsg_frame_calc_checksum(tx_msg, sizeof(m33_to_m85_msg_t));

    rpmsg_frame_print_m33_to_m85(tx_msg, "TX");

    if (rt_mb_send(rpmsg_comm.tx_mb, (rt_uint32_t)tx_msg) != RT_EOK)
    {
        LOG_E("Failed to send message to tx mailbox!");
        rt_free(tx_msg);
        return -RT_ERROR;
    }

    return RT_EOK;
}
MSH_CMD_EXPORT(rpmsg_send_to_core0, send message from M33);

rt_err_t rpmsg_send_to_core1(int argc, char *argv[])
{
    if (argc != 4)
    {
        rt_kprintf("Usage: rpmsg_send_to_core1 <index> <vehicle_speed> <steering_angle>\n");
        rt_kprintf("Example: rpmsg_send_to_core1 1 12.5 45.0\n");
        return -RT_ERROR;
    }

    for (int i = 1; i < argc; i++)
    {
        if (argv[i] == RT_NULL || strlen(argv[i]) == 0)
        {
            rt_kprintf("Error: Argument %d is invalid\n", i);
            return -RT_ERROR;
        }
    }

    unsigned long index_value = 0;
    rt_uint32_t index = 0;
    float vehicle_speed = 0.0f;
    float steering_angle = 0.0f;

    if (sscanf(argv[1], "%lu", &index_value) != 1)
    {
        rt_kprintf("Error: Invalid index format\n");
        return -RT_ERROR;
    }
    index = (rt_uint32_t)index_value;

    vehicle_speed = (float) atof(argv[2]);
    steering_angle = (float) atof(argv[3]);

    m85_to_m33_msg_t *tx_msg = rt_malloc(sizeof(m85_to_m33_msg_t));
    if (!tx_msg)
    {
        LOG_E("Failed to allocate memory for tx message!");
        return -RT_ENOMEM;
    }
    rt_memset(tx_msg, 0, sizeof(*tx_msg));
    tx_msg->type = RPMSG_M85_TO_M33_TYPE_CONTROL;

    tx_msg->index = index;
    tx_msg->vehicle_speed = vehicle_speed;
    tx_msg->steering_angle = steering_angle;
    tx_msg->checksum = rpmsg_frame_calc_checksum(tx_msg, sizeof(m85_to_m33_msg_t));

    rpmsg_frame_print_m85_to_m33(tx_msg, "TX");

    if (rt_mb_send(rpmsg_comm.tx_mb, (rt_uint32_t)tx_msg) != RT_EOK)
    {
        LOG_E("Failed to send message to tx mailbox!");
        rt_free(tx_msg);
        return -RT_ERROR;
    }

    return RT_EOK;
}
MSH_CMD_EXPORT(rpmsg_send_to_core1, send message from M85);

rt_err_t rpmsg_send_ready_to_core0(void)
{
    m33_to_m85_msg_t *tx_msg;

    if (rpmsg_comm.tx_mb == RT_NULL)
    {
        return -RT_ERROR;
    }

    tx_msg = (m33_to_m85_msg_t *)rt_malloc(sizeof(m33_to_m85_msg_t));
    if (!tx_msg)
    {
        return -RT_ENOMEM;
    }
    rt_memset(tx_msg, 0, sizeof(*tx_msg));

    tx_msg->index = 0;
    tx_msg->type = RPMSG_M33_TO_M85_TYPE_READY;
    tx_msg->checksum = rpmsg_frame_calc_checksum(tx_msg, sizeof(*tx_msg));

    if (rt_mb_send(rpmsg_comm.tx_mb, (rt_uint32_t)tx_msg) != RT_EOK)
    {
        rt_free(tx_msg);
        return -RT_ERROR;
    }

    return RT_EOK;
}