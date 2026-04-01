/*
 * Copyright (c) 2006-2026, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-01-02     yuanjie     first version
 */

#include <rtthread.h>
#include <ospi_b_commands.h>
#include "board.h"
#include "hal_data.h"
#include "drv_common.h"

#if defined(RT_USING_FAL)

#include "fal.h"

//#define DRV_DEBUG
#define DBG_TAG "drv.w25qxx"
#ifdef DRV_DEBUG
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_INFO
#endif /* DRV_DEBUG */
#include <rtdbg.h>

/* Flash device sector size */
#define OSPI_B_SECTOR_SIZE_4K               (0x1000)
#define OSPI_B_SECTOR_SIZE_256K             (0x40000)
#define OSPI_B_SECTOR_4K_END_ADDRESS        (0x9001FFFF)

/* Flash device timing */
#define OSPI_B_TIME_UNIT                    (BSP_DELAY_UNITS_MICROSECONDS)
#define OSPI_B_TIME_RESET_SETUP             (2U)             //  Type 50ns
#define OSPI_B_TIME_RESET_PULSE             (1000U)          //  Type 500us
#define OSPI_B_TIME_ERASE_256K              (1500000U)       //  Type 256KB sector is 331 KBps -> Type 0.773s
#define OSPI_B_TIME_ERASE_4K                (100000U)        //  Type 4KB sector is 95 KBps -> Type 0.042s
#define OSPI_B_TIME_WRITE                   (10000U)         //  Type 256B page (4KB/256KB) is 595/533 KBps -> Type

/* Flash device status bit */
#define OSPI_B_WEN_BIT_MASK                 (0x00000002)
#define OSPI_B_BUSY_BIT_MASK                (0x00000001)

#define OSPI_B_CS1_START_ADDRESS            (0x90000000)
#define OSPI_B_APP_ADDRESS(sector_no)       ((uint8_t *)(OSPI_B_CS1_START_ADDRESS + ((sector_no) * OSPI_B_SECTOR_SIZE_4K)))

#define RENESAS_FLASH_START_ADDRESS         OSPI_B_CS1_START_ADDRESS
#define RENESAS_FLASH_SIZE                  (8 * 1024 * 1024) /* 8MB for W25Q64 flash */
#define RENESAS_FLASH_SECTOR_SIZE           OSPI_B_SECTOR_SIZE_4K /* 4KB sectors */
#define RENESAS_FLASH_END_ADDRESS           (RENESAS_FLASH_START_ADDRESS + RENESAS_FLASH_SIZE)

/* External variables */
extern spi_flash_direct_transfer_t g_ospi_b_direct_transfer[OSPI_B_TRANSFER_MAX];

/**
 * @brief       This functions enables write and verify the read data.
 * @param       None
 * @retval      FSP_SUCCESS     Upon successful operation
 * @retval      FSP_ERR_ABORTED Upon incorrect read data.
 * @retval      Any Other Error code apart from FSP_SUCCESS Unsuccessful operation
 */
static fsp_err_t ospi_b_write_enable(void)
{
    fsp_err_t err = FSP_SUCCESS;
    spi_flash_direct_transfer_t transfer = {0};

    transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_WRITE_ENABLE_SPI];
    err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    if (FSP_SUCCESS != err)
    {
        LOG_E("R_OSPI_B_DirectTransfer API FAILED");
        return err;
    }

    transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_STATUS_SPI];
    err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    if (FSP_SUCCESS != err)
    {
        LOG_E("R_OSPI_B_DirectTransfer API FAILED");
        return err;
    }

    /* Check Write Enable bit in Status Register */
    if (OSPI_B_WEN_BIT_MASK != (transfer.data & OSPI_B_WEN_BIT_MASK))
    {
        LOG_E("Write enable FAILED");
        return FSP_ERR_ABORTED;
    }
    return err;
}

/**
 * @brief       This function wait until OSPI operation completes.
 * @param[in]   timeout         Maximum waiting time
 * @retval      FSP_SUCCESS     Upon successful wait OSPI operating
 * @retval      FSP_ERR_TIMEOUT Upon time out
 * @retval      Any Other Error code apart from FSP_SUCCESS Unsuccessful operation.
 */
static fsp_err_t ospi_b_wait_operation(uint32_t timeout)
{
    fsp_err_t err = FSP_SUCCESS;
    spi_flash_status_t status = {0};

    status.write_in_progress = true;
    while (status.write_in_progress)
    {
        /* Get device status */
        R_OSPI_B_StatusGet(&g_ospi_b_ctrl, &status);
        if (FSP_SUCCESS != err)
        {
            LOG_E("R_OSPI_B_StatusGet API FAILED");
            return err;
        }
        if (0 == timeout)
        {
            LOG_E("OSPI time out occurred");
            return FSP_ERR_TIMEOUT;
        }
        rt_thread_mdelay(1);
        timeout--;
    }
    return err;
}

/**
 * @brief init w25q flash in spi mode using default RA HAL
 *
 * @return int
 * @todo QSPI mode testing
 */
int w25q64_init(void)
{
    fsp_err_t err = FSP_SUCCESS;
    spi_flash_direct_transfer_t transfer = {0};
    // @todo page size settings
    /* Open OSPI module */
    err = R_OSPI_B_Open(&g_ospi_b_ctrl, &g_ospi_b_cfg);
    if (FSP_SUCCESS != err)
    {
        LOG_E("failed to open device");
    }

    /* Switch OSPI module to 1S-1S-1S mode to configure flash device */
    err = R_OSPI_B_SpiProtocolSet(&g_ospi_b_ctrl, SPI_FLASH_PROTOCOL_EXTENDED_SPI);
    if (err != FSP_SUCCESS)
    {
        LOG_E("failed to probe SPI mode");
    }

    transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_DEVICE_ID_SPI];
    err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    if (err != FSP_SUCCESS)
    {
        LOG_E("failed to read Flash id");
    }
    LOG_D("FLASH ID:%#x, len:%d", transfer.data, transfer.data_length);

    /*-------- switch to 1S-4S-4S mode ---------*/
//    // read SR2
//    transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_SR2_SPI];
//    err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
//    if (err != FSP_SUCCESS)
//    {
//        LOG_E("unable to read status register2!");
//        return err;
//    }
//
//    uint8_t sr2 = (uint8_t)transfer.data;
//
//    // set QE bit
//    if (!(sr2 & 0x02))
//    {
//        sr2 |= 0x02; // QE 位在 SR2 Bit1
//
//        // write enable
//        err = ospi_b_write_enable();
//        if (err != FSP_SUCCESS)
//        {
//            LOG_E("Write enable failed before SR2 write");
//            return err;
//        }
//
//        transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_WRITE_SR2_SPI];
//        transfer.data = sr2;
//        err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
//        if (err != FSP_SUCCESS)
//        {
//            LOG_E("unable to write status register2!");
//            return err;
//        }
//
//        // wait done
//        err = ospi_b_wait_operation(OSPI_B_TIME_WRITE);
//        if (err != FSP_SUCCESS)
//            return err;
//    }
//
//    rt_thread_mdelay(200);
//
//    err = R_OSPI_B_SpiProtocolSet(&g_ospi_b_ctrl, SPI_FLASH_PROTOCOL_1S_4S_4S);
//    if (err != FSP_SUCCESS)
//    {
//        LOG_E("failed to probe QSPI mode");
//        return err;
//    }
//
//    transfer = g_ospi_b_direct_transfer[OSPI_B_TRANSFER_READ_DEVICE_ID_OPI];
//    err = R_OSPI_B_DirectTransfer(&g_ospi_b_ctrl, &transfer, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
//    if (err != FSP_SUCCESS)
//    {
//        LOG_E("failed to read Flash id");
//    }
//    LOG_D("FLASH ID:%#x, len:%d", transfer.data, transfer.data_length);

    LOG_D("norflash init success");
    return 0;
}

/**
 * @brief erase flash in sector region
 *
 * @param addr
 * @param size
 * @return fsp_err_t FSP_SUCCESS or other errors
 */
static int w25q64_erase(long offset, size_t size)
{
    fsp_err_t err;
    rt_uint32_t addr = RENESAS_FLASH_START_ADDRESS + offset;
    rt_uint32_t end_addr = addr + size;
    rt_uint32_t sector_count;
    rt_uint32_t sector_no;

    if (end_addr > RENESAS_FLASH_END_ADDRESS)
    {
        LOG_E("erase outrange flash size! addr is (0x%p)", (void *)end_addr);
        return -RT_EINVAL;
    }

    /* Calculate starting sector and number of sectors to erase */
    sector_no = offset / RENESAS_FLASH_SECTOR_SIZE;
    sector_count = (size + RENESAS_FLASH_SECTOR_SIZE - 1) / RENESAS_FLASH_SECTOR_SIZE; /* Ceiling division */

    for (rt_uint32_t i = 0; i < sector_count; i++)
    {
        rt_uint32_t sector_addr = (rt_uint32_t)OSPI_B_APP_ADDRESS(sector_no + i);

        /* Perform sector erase */
        err = R_OSPI_B_Erase(&g_ospi_b_ctrl, (uint8_t *)sector_addr, RENESAS_FLASH_SECTOR_SIZE);
        if (err != FSP_SUCCESS)
        {
            LOG_E("OSPI erase failed at addr 0x%08x, error code: %d", sector_addr, err);
            return -RT_ERROR;
        }

        /* Wait for erase completion */
        err = ospi_b_wait_operation(OSPI_B_TIME_ERASE_4K);
        if (err != FSP_SUCCESS)
        {
            LOG_E("OSPI wait operation failed for sector %d, error code: %d", sector_no + i, err);
            return -RT_ERROR;
        }
    }

    return (int)size;
}

/**
 * @brief Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 * @todo word aligned issue
 */
static int w25q64_read(long offset, uint8_t *buf, size_t size)
{
    rt_uint32_t addr = RENESAS_FLASH_START_ADDRESS + offset;

    if ((addr + size) > RENESAS_FLASH_END_ADDRESS)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return -RT_EINVAL;
    }

#if defined(__DCACHE_PRESENT)
    // Invalidate cache for the read region to ensure data consistency
    SCB_InvalidateDCache_by_Addr((uint32_t *)addr, size);
#endif
    /* Directly read from memory-mapped address in XIP mode */
    rt_memcpy(buf, (void *)addr, size);

    return (int)size;
}

/**
 * @brief This function performs an write operation on the flash device.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr Pointer to flash device memory address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
static int w25q64_write(long offset, const uint8_t *buf, size_t size)
{
    fsp_err_t err;
    rt_uint32_t addr = RENESAS_FLASH_START_ADDRESS + offset;
    rt_uint32_t remaining = size;
    rt_uint8_t *p_buf = (rt_uint8_t *)buf;
    const rt_uint32_t chunk_size = 4; // Fixed 4-byte write size

    // Input validation
    if (!buf || size == 0)
    {
        LOG_E("Invalid input: buf=%p, size=%u", buf, size);
        return -RT_EINVAL;
    }

    if ((addr + size) > RENESAS_FLASH_END_ADDRESS)
    {
        LOG_E("Write out of range: addr=0x%08x, size=%u", addr, size);
        return -RT_EINVAL;
    }

    // Write data in 4-byte chunks
    while (remaining > 0)
    {
        // Calculate current chunk size (up to 4 bytes or remaining bytes)
        rt_uint32_t current_size = (remaining >= chunk_size) ? chunk_size : remaining;

        // Perform write operation
        err = R_OSPI_B_Write(&g_ospi_b_ctrl, p_buf, (uint8_t *)addr, current_size);
        if (err != FSP_SUCCESS)
        {
            LOG_E("OSPI write failed: addr=0x%08x, size=%u, error=%d", addr, current_size, err);
            return -RT_ERROR;
        }

#if defined(__DCACHE_PRESENT)
        // Clean and invalidate cache for the written region
        SCB_CleanInvalidateDCache_by_Addr((uint32_t *)addr, current_size);
#endif

        // Wait for write operation to complete
        err = ospi_b_wait_operation(OSPI_B_TIME_WRITE);
        if (err != FSP_SUCCESS)
        {
            LOG_E("OSPI wait failed: addr=0x%08x, error=%d", addr, err);
            return -RT_ERROR;
        }

        // Update address, buffer pointer, and remaining bytes
        addr += current_size;
        p_buf += current_size;
        remaining -= current_size;
    }

    return (int)size;
}

const struct fal_flash_dev w25q64 =
    {
        .name = "W25Q64",
        .addr = 0,
        .len = RENESAS_FLASH_SIZE,
        .blk_size = RENESAS_FLASH_SECTOR_SIZE,
        .ops = {
            .init = w25q64_init,
            .read = w25q64_read,
            .write = w25q64_write,
            .erase = w25q64_erase},
        .write_gran = 0,
};
#endif /* RT_USING_FAL */
