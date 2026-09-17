/* Project settings retained by RT-Thread menuconfig header generation. */
#ifndef TITAN_OPENMV_RTCONFIG_PROJECT_H
#define TITAN_OPENMV_RTCONFIG_PROJECT_H
/* This project maintains only the OpenMV + NPU configuration. */
#define TITAN_OPENMV_SRAM_GC_BYTES 1376256
#define BSP_USING_OPENMV_ML
#define BSP_USING_OPENMV_NPU
#define OMV_ENABLE_TF
#if defined(BSP_USING_OPENMV) && (defined(BSP_OPENMV_STORAGE_SD) + defined(BSP_OPENMV_STORAGE_FLASH) != 1)
#error "Select exactly one OpenMV storage medium in menuconfig: SD or SPI Flash"
#endif
#if defined(BSP_USING_OPENMV) && defined(BSP_USING_FILESYSTEM)
#error "OpenMV owns FAT mounting; disable the SDK filesystem initialization"
#endif
#if defined(BSP_OPENMV_STORAGE_FLASH) && (defined(BSP_USING_QSPI_FLASH) || defined(RT_USING_FAL))
#error "OpenMV SPI Flash uses its project driver; disable SDK QSPI/FAL drivers"
#endif
#if defined(BSP_OPENMV_STORAGE_FLASH) && (defined(BSP_USING_SDHI) || defined(RT_USING_SDIO))
#error "OpenMV SPI Flash storage must not enable the SD host"
#endif
#if defined(BSP_OPENMV_STORAGE_SD) && (!defined(BSP_USING_SDHI0) || !defined(RT_USING_SDIO))
#error "OpenMV SD storage requires SDHI0 and RT-Thread MMC/SD"
#endif
#if defined(BSP_USING_OPENMV) && !defined(RT_USING_DFS_V1)
#error "Titan Mini OpenMV requires DFS v1; restore RT_USING_DFS_V1 in menuconfig"
#endif
#if defined(PKG_USING_TINYUSB) && defined(BSP_USING_RA8P1_USB)
#error "Titan Mini OpenMV uses TinyUSB; disable the generic FSP USB driver"
#endif
#if defined(BSP_USING_OPENMV) && defined(BSP_USING_CEU_CAMERA)
#error "Titan Mini OpenMV uses MIPI CSI; disable CEU"
#endif
#endif
