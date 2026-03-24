# 模板工程

**中文** | [**English**](README.md)

## 简介

这是 **Titan Board Mini** 的模板工程。它实现了基本的 LED 闪烁功能，可以作为二次开发的起点。

## 特性

本模板提供：
- RT-Thread 完整系统配置
- 基础 LED 驱动（GPIO）
- 串口调试输出（UART）
- 二次开发项目结构

## 项目结构

```
project/
├── board/              # 板级配置
├── libraries/          # RA8P1 HAL 库
├── rt-thread/          # RT-Thread 内核
├── src/                # 应用源代码
├── Kconfig             # Menuconfig 配置
├── SConstruct          # 构建脚本
└── ...
```

## 快速开始

1. 在 RT-Thread Studio 中打开工程
2. 如需要，使用 menuconfig 配置工程
3. 编译工程
4. 下载并运行

## 二次开发

本模板可作为以下开发的基础：
- 添加新的外设
- 实现自定义应用
- 开发驱动程序
- 构建完整应用

## 进阶阅读

- [RT-Thread 文档](https://www.rt-thread.org/document/site/)
