# 存储与文件管理

根文件系统与 USB MSC 共用一个存储介质。编译配置选择 SD 卡或板载 W25Q64 SPI Flash，默认 SD；运行时固定使用所选介质。

## 配置

在工程目录执行 `python -m SCons --pyconfig`，进入：

```text
Titan Mini OpenMV port
  OpenMV filesystem and USB MSC medium
    SD card (SDHI0)
    Onboard SPI Flash (W25Q64, 6 MiB)
```

保存配置后执行 [编译和收集](development.md)，再烧录 `firmware/rtthread.hex` 并复制配套视觉 BIN。切换后端时手动迁移应用数据。构建信息位于 `build/reports/manifest.json`，其中 `storage.backend` 为 `sd` 或 `spi-flash`。

| 项目 | SD 卡（默认） | 板载 SPI Flash |
|---|---|---|
| 配置 | `BSP_OPENMV_STORAGE_SD` | `BSP_OPENMV_STORAGE_FLASH` |
| 块设备 | `sd` | `qflash` |
| 容量 | 从实际 SD 卡读取 | 固定 6 MiB |
| 首次准备 | 电脑上预先准备 FAT / FAT32 | 明确无 FAT 且无读写错误时自动初始化一次 |
| 自动格式化 | 不执行 | 每次启动最多一次，MSC 公开后不再执行 |
| 适用情况 | 大模型、图片、录像及完整示例素材 | 少量模型、无需 SD 卡的应用 |

## 路径

板端 `/` 与电脑看到的 U 盘根目录对应。`/rom` 是同一目录的只读别名；例如 `/rom/palm_detection_full_192_u55_256.tflite` 对应根目录的 `palm_detection_full_192_u55_256.tflite`。从 `models/sdcard/` 复制模型时保持原文件名，无需创建 `rom` 或 `sdcard` 子目录。

```text
存储卷根目录/
├── openmv-vision.bin         与 HEX 配套的视觉资源
├── main.py                  可选：硬件启动时运行
├── boot.py                  可选：启动初始化
├── lib/                     可选：应用导入的 Python 模块
├── *.tflite、*.txt           应用所需模型与标签
└── omv_assets/               示例图片等素材，按示例要求复制
```

视觉文件由固件解压加载。固件更新保留有效文件系统；更新视觉文件后执行硬件复位。

## 文件传输

**电脑和板端同时写入 FAT 卷可能损坏文件系统。** 按以下顺序交替访问：

1. 停止板端写文件脚本。
2. 在电脑端复制文件，等待复制结束。
3. 使用操作系统“安全弹出”完成主机缓存写回。
4. 再运行板端文件读写程序。

安全弹出后 CDC 保持连接，板端 FAT 保持挂载。介质导出接口见 [USB](usb.md)。文件持久化检查使用 [文件系统示例](../examples/04_系统与存储/03_文件系统/)：写入后硬件复位，再次读取并核对内容。

## SD 卡

- 固件导出整卡块设备，FatFs 识别已有 FAT 分区。支持 FAT / FAT32，不支持 exFAT；格式转换前备份数据。
- 无卡上电时 CDC 与基础 Python 可用，MSC 报告无介质；首次插卡可被初始化。
- 已枚举的卡拔出后保持无介质状态，换卡必须复位。先停止两端文件访问并安全弹出，再断电换卡。
- SD 配置不会初始化或擦写板载 W25Q64。

## SPI Flash

W25Q64 通过 OSPI0 / CS1 单线 SPI 手动命令读写，内存映射关闭。

| Flash 地址 | 用途 |
|---|---|
| `0x000000–0x1FFFFF` | 前 2 MiB 保留，本存储驱动不读写 |
| `0x200000–0x7FFFFF` | 后 6 MiB 用作 OpenMV FAT 卷 |

逻辑扇区 512 字节，共 12,288 个；擦除单位 4 KiB。新建文件系统为 FAT16。已有有效 FAT 的内容会保留，即使原卷容量较小。

**首次迁移到 Flash 固件前，备份后 6 MiB 的数据。** 探测结果为无 FAT 且无 I/O 错误时，驱动执行格式化；LittleFS 等非 FAT 数据会被清除。每次启动最多尝试一次，MSC 导出后停止自动格式化。

6 MiB 容量由视觉文件、模型和应用数据共用。U-Net 宠物分割模型与视觉文件的总大小超出该容量，需使用 SD。Flash 驱动未实现磨损均衡和掉电日志；频繁数据记录使用 SD，写入期间保持供电。

## 诊断

命令入口：UART1，RT-Thread `msh`。

| 命令 | 用途 |
|---|---|
| `list device` | 查看 `sd` 或 `qflash` 块设备 |
| `mount`、`ls` | 查看挂载与根目录 |
| `storage_mount` | 重试初始化和挂载；也提供 `sd_mount` 或 `flash_mount` 别名 |
| `usb_msc status` | 查看主机介质状态 |

`mount` 显示挂载状态，介质读写状态需结合操作返回值和启动日志判断。更换介质后执行硬件复位，再检查挂载和文件读写。
