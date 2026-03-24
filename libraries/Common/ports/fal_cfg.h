/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-20     Sherman      the first version
 */
#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include "hal_data.h"
#include "rtconfig.h"

extern const struct fal_flash_dev w25q64;

/* flash device table */
#define FAL_FLASH_DEV_TABLE             \
{                                       \
    &w25q64,                            \
}
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/** partition table, The chip flash partition is defined in "\ra\fsp\src\bsp\mcu\ra6m4\bsp_feature.h".
 * More details can be found in the RA6M4 Group User Manual: Hardware section 47 Flash memory.*/
#define FAL_PART_TABLE                                                                        \
{                                                                                             \
    {FAL_PART_MAGIC_WORD, "bootloader", "W25Q64",                         0,  512 * 1024, 0}, \
    {FAL_PART_MAGIC_WORD,        "app", "W25Q64",                512 * 1024,  512 * 1024, 0}, \
    {FAL_PART_MAGIC_WORD,   "download", "W25Q64",        (512 + 512) * 1024, 1024 * 1024, 0}, \
    {FAL_PART_MAGIC_WORD, "filesystem", "W25Q64", (512 + 512 + 1024) * 1024, 1024 * 1024, 0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */
#endif /* _FAL_CFG_H_ */

