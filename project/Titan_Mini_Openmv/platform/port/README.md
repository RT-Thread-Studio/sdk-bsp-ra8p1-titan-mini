# 平台适配源码

本目录保存已固化的 Titan Mini OpenMV 适配源码和配置，直接参与 SCons 编译，
不再通过一次性转换脚本生成。文件保留上游许可证，可直接维护；升级 RT-Thread、
OpenMV、MicroPython、TinyUSB 或 RASC 配置时，需要同步检查对应适配。

- `dfs_*`、`titan_sdhi_*` 与 `titan_mmcsd_core.c`：文件系统及 SD 驱动适配。
- `dcd_rusb2_ra8.c`、`rusb2_ra8.h`、`usbd_ra8.c`、`msc_ra8.c`：RA8 USB 适配。
- `titan_*`、`imlib_titan.h`、`omv_protocol_channel_stdio_titan.c`：VM、图像、推理和通信适配。
- `common_data_openmv.c`、`r_mipi_csi.c`：VIN DMA 配置和 MIPI 接收状态修正。
- `vector_data_openmv.c`、`machine_cfg/`、`titan_machine_vectors.h`：配套维护的中断向量。
  I2C1 使用原 Ethernet 向量槽 0～2，启用 Ethernet 前必须重新分配。
- `flash_cfg/`：Flash 后端专用 OSPI 和时钟配置；SD 构建使用原配置。
- `storage_kconfig/`：保留 SD/Flash 互斥选择及 SDK 存储驱动约束的 Kconfig。

`scons` 自动生成的 QSTR、模块注册、冻结模块和链接段列表位于 `build/generated/`。
清理构建目录不会删除这里的发布源码。
