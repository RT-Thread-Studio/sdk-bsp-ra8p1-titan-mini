/* SPDX-License-Identifier: Apache-2.0 */
#include <stdint.h>
#include <rtthread.h>
#include "hal_data.h"
#include "ethosu_driver.h"
#include "titan_memory.h"
#include "titan_npu.h"

static unsigned int initialization_state;

/* The OpenMV VM is the owner of model initialization and invocation. The
 * registered driver survives Ctrl-D resets; TFLM releases it after each call. */
int titan_npu_init(void)
{
    /* FSP parameter checks are disabled in this BSP: ALREADY_OPEN is not
     * guaranteed. Reopening would register the same core-driver list node
     * twice. Serialize ownership without holding a lock across allocation. */
    rt_base_t level = rt_hw_interrupt_disable();
    if (initialization_state != 0) {
        unsigned int state = initialization_state;
        rt_hw_interrupt_enable(level);
        return state == 2 ? 0 : -RT_EBUSY;
    }
    initialization_state = 1;
    rt_hw_interrupt_enable(level);
    fsp_err_t result = RM_ETHOSU_Open(&g_rm_ethosu0_ctrl, &g_rm_ethosu0_cfg);
    level = rt_hw_interrupt_disable();
    initialization_state = result == FSP_SUCCESS ? 2 : 0;
    rt_hw_interrupt_enable(level);
    return result == FSP_SUCCESS ? 0 : -(int)result;
}

/* Return to Python only after completion or a confirmed hardware reset.
 * The vendor driver ignores reset failure after a timeout; retaining this
 * boundary prevents GC/soft-reset from recycling memory still owned by DMA. */
int titan_npu_invoke(struct ethosu_driver *driver, const void *commands, int command_size,
                     uint64_t *const addresses, const size_t *sizes, int count, void *user)
{
    if (!driver || command_size <= 0 || count <= 0 || count > 8 || !addresses || !sizes ||
        !titan_memory_is_bus_accessible(commands, (size_t)command_size)) {
        return -1;
    }
    for (int i = 0; i < count; ++i) {
        /* Vela may emit an unused, zero-length fast-scratch tensor with a
         * null base. Only nonempty tensor ranges require a DMA address. */
        if (addresses[i] > UINTPTR_MAX || (sizes[i] != 0 &&
            !titan_memory_is_bus_accessible((void *)(uintptr_t)addresses[i], sizes[i]))) {
            return -1;
        }
    }

    titan_cache_clean(commands, (size_t)command_size);

    int result = ethosu_invoke_v3(driver, commands, command_size, addresses, sizes, count, user);
    if (result != 0) {
        R_BSP_IrqDisable(g_rm_ethosu0_cfg.irq);
        if (ethosu_soft_reset(driver) != 0) {
            /* A failed peripheral reset cannot provide DMA ownership back.
             * System reset is the terminal recovery; never expose the arena. */
            rt_kprintf("NPU reset failed; restarting to protect DMA buffers\n");
            NVIC_SystemReset();
            for (;;) {}
        }
        R_BSP_IrqStatusClear(g_rm_ethosu0_cfg.irq);
        NVIC_ClearPendingIRQ(g_rm_ethosu0_cfg.irq);
        while (rt_sem_take((rt_sem_t)driver->semaphore, 0) == RT_EOK) {}
        R_BSP_IrqEnable(g_rm_ethosu0_cfg.irq);
    }
    /* Vendor 0.16 invalidates before its blocking semaphore wait. Repeat
     * after completion so no speculative/stale output line survives. */
    for (int i = 0; i < count; ++i) {
        if (driver->basep_invalidate_mask & (1U << i)) {
            titan_cache_invalidate((void *)(uintptr_t)addresses[i], sizes[i]);
        }
    }

    return result;
}

void *ethosu_mutex_create(void) { return rt_mutex_create("npu", RT_IPC_FLAG_PRIO); }
void ethosu_mutex_destroy(void *mutex) { if (mutex) rt_mutex_delete((rt_mutex_t)mutex); }
int ethosu_mutex_lock(void *mutex) { return rt_mutex_take((rt_mutex_t)mutex, RT_WAITING_FOREVER); }
int ethosu_mutex_unlock(void *mutex) { return rt_mutex_release((rt_mutex_t)mutex); }
void *ethosu_semaphore_create(void) { return rt_sem_create("npu", 0, RT_IPC_FLAG_PRIO); }
void ethosu_semaphore_destroy(void *sem) { if (sem) rt_sem_delete((rt_sem_t)sem); }
int ethosu_semaphore_take(void *sem, uint64_t timeout)
{
    /* The core driver declares timeout units implementation-defined. Our
     * finite values are milliseconds; WAIT_FOREVER remains unbounded only
     * for driver reservation. Inference uses the 5000 ms build-time limit. */
    rt_int32_t ticks = RT_WAITING_FOREVER;
    if (timeout != ETHOSU_SEMAPHORE_WAIT_FOREVER) {
        if (timeout > INT32_MAX) timeout = INT32_MAX;
        ticks = rt_tick_from_millisecond((rt_int32_t)timeout);
    }
    return rt_sem_take((rt_sem_t)sem, ticks);
}
int ethosu_semaphore_give(void *sem) { return rt_sem_release((rt_sem_t)sem); }

void ethosu_flush_dcache(uint32_t *address, size_t size)
{
    if (address) titan_cache_clean(address, size);
    else SCB_CleanDCache();
}
void ethosu_invalidate_dcache(uint32_t *address, size_t size)
{
    /* The driver supplies concrete base pointer ranges. Do not clean a
     * completed NPU output: cached CPU data may predate the DMA result. */

    if (address) titan_cache_invalidate(address, size);
    else SCB_InvalidateDCache();
}
