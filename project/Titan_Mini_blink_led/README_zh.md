# LED 闪烁示例

**中文** | [**English**](README.md)

## 简介

本示例是 SDK 中最简单、最基础的示例，类似于程序员接触的第一个程序 Hello World 一样简洁。它的主要功能是让板载的 RGB LED 进行周期性闪烁。

## GPIO 简介

### 1. 概述

**GPIO（通用输入输出）** 是 MCU 最常用的外设接口之一，能够在软件控制下配置为 **输入模式** 或 **输出模式**：

- **输入模式**：用于读取外部电平状态，例如按键输入。
- **输出模式**：用于控制外设电平，例如点亮 LED、驱动蜂鸣器。

### 2. RA8 系列 GPIO 特点

- **丰富的引脚资源**：每个端口可独立配置为输入/输出/复用功能。
- **多功能复用**：除 GPIO 功能外，同一引脚还可能作为 UART、SPI、I2C 等外设的信号线。
- **电平控制**：输出时可驱动高电平或低电平，部分引脚支持开漏模式。
- **方向控制**：输入/输出可动态切换。

### 3. RT-Thread PIN 框架

RT-Thread 提供了 **PIN 设备驱动框架**，通过统一的接口屏蔽底层硬件差异：

- `rt_pin_mode()` - 设置引脚工作模式（输入/输出/上拉/下拉等）
- `rt_pin_write()` - 输出电平（高/低）
- `rt_pin_read()` - 读取输入电平

这样开发者不需要直接操作寄存器，而是通过 RT-Thread 的 API 即可完成 GPIO 控制。

在本示例中，LED 引脚被配置为 **输出模式**，软件循环输出高低电平，从而实现 LED 闪烁。

## 硬件说明

Titan Board Mini 提供三个用户 LED：LED_R（红色）、LED_B（蓝色）、LED_G（绿色）。LED 对应 MCU 上的特定引脚。MCU 引脚输出低电平即可点亮 LED，输出高电平则会熄灭 LED。

## 软件说明

本例程的源码位于 `src/hal_entry.c`。RGB LED 对应的单片机引脚定义及 LED 控制逻辑可以查阅源码文件。

```c
/* 配置 LED 灯引脚 */
rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

/* 控制 LED */
rt_pin_write(LED_PIN, LED_ON);  // 点亮
rt_pin_write(LED_PIN, LED_OFF); // 熄灭
```

## 编译下载

1. 在 RT-Thread Studio 中打开工程
2. 点击构建按钮进行编译
3. 连接调试器到开发板
4. 下载固件到开发板

## 运行效果

按下复位按键重启开发板，观察 LED 闪烁效果。RGB LED 会周期性变化。

## 进阶阅读

- [RT-Thread PIN 设备](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/pin/pin)