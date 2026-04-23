# sdk-bsp-ra8p1-titan-board-mini

**中文** | [**English**](README.md)

## 简介

中文页

sdk-bsp-ra8p1-titan-board-mini 是 RT-Thread 团队对 `Titan Board Mini` 开发板所作的支持包，也可作为用户开发使用的软件SDK，让用户可以更简单方便的开发自己的应用程序。

Titan Board Mini开发板是 RT-Thread 推出基于瑞萨 Cortex-M85 与 Cortex-M33 双核架构 R7KA8P1 芯片，为工程师们提供了一个灵活、全面的开发平台，助力开发者在嵌入式物联网领域获得更深层次的体验。

![img](figures/big.png)

## 目录结构

```
$ sdk-bsp-ra8p1-titan-board-mini
├── docs
│   ├── Titan_Mini_datasheet.pdf
│   ├── ra8p1-mini_v0.1.pdf
│   └── Titan_Mini_user_manual.pdf
├── figures
├── libraries
├── project
│   ├── Titan_Mini_blink_led
│   ├── Titan_Mini_component_flash_fs
│   ├── Titan_Mini_display_camera_mipi_csi
│   ├── Titan_Mini_display_rgb_lvgl
│   ├── Titan_Mini_driver_adc
│   ├── Titan_Mini_driver_canfd
│   ├── Titan_Mini_driver_eth
│   ├── Titan_Mini_driver_gpt
│   ├── Titan_Mini_driver_rtc
│   ├── Titan_Mini_driver_sdcard
│   ├── Titan_Mini_driver_sdram
│   ├── Titan_Mini_driver_spi
│   ├── Titan_Mini_pdm
│   ├── Titan_Mini_driver_wdt
│   ├── Titan_Mini_key_irq
│   ├── Titan_Mini_peripheral_imu
│   ├── Titan_Mini_rgb_lcd
│   ├── Titan_Mini_template
│   ├── Titan_Mini_usb_pcdc
│   └── Titan_Mini_wavplayer
├── rt-thread
├── README.md
├── README_zh.md
└── sdk-bsp-ra8p1-titan-board-mini.yaml
```

- sdk-bsp-ra8p1-titan-board-mini.yaml：描述 Titan Board Mini 的硬件信息
- docs：开发板原理图，文档以及 datasheets 等
- libraries ：Titan Board Mini 通用外设驱动
- project：示例工程文件夹
- rt-thread： rt-thread 源码

## 使用方式

`sdk-bsp-ra8p1-titan-board-mini` 支持 **RT-Thread Studio** 开发方式

## **RT-Thread Studio 开发步骤**

1. 打开 RT-Thread Studio ，安装 Titan Board Mini 开发板支持包（如有最新建议安装最新版本，下图版本仅供参考）

2. 新建 Titan Board Mini 工程，选择左上角文件->新建->RT-Thread 项目->基于开发板，可以创建示例工程或模板工程

![image-create](figures/create.png)

3. 进行工程的编译和下载：

<img src="figures/image-20250820093329972.png" alt="image-20250820093329972" style="zoom: 200%;" />
