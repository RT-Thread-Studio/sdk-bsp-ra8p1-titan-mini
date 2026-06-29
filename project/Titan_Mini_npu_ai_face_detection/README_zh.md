# MIPI NPU 人脸检测

[English](./README.md) | [中文](./README_zh.md)

## 简介

该示例在 **Titan Board Mini** 上实现了基于 **MIPI CSI 摄像头** 的实时人脸检测，主要使用以下模块：

- **OV5640** 通过 **MIPI CSI** 接入
- **RA8P1 VIN** 负责图像采集
- **Arm Ethos-U55 NPU** 负责 YOLO-Fastest 推理
- **RGB565 LCD** 负责实时预览和人脸框叠加显示

## 功能

- 通过 **MIPI CSI + VIN** 采集摄像头图像
- 使用 **YOLO-Fastest 人脸检测模型** 进行 NPU 推理
- 在 LCD 上叠加绿色人脸框
- 支持通过用户按键触发 **OV5640 自动对焦**
- 支持通过宏定义控制检测日志输出

## 数据流

```text
OV5640
  -> MIPI CSI
  -> VIN 帧缓冲
  -> CPU 预处理（缩放 + 灰度化 + 量化）
  -> Ethos-U55 推理
  -> CPU 后处理（解码 + NMS）
  -> D/AVE2D 叠框
  -> GLCDC
  -> RGB565 LCD
```

## 模型信息

- 模型：**YOLO-Fastest 人脸检测**
- 框架：**TensorFlow Lite INT8**
- 模型输入尺寸：**192 x 192**
- 摄像头采集尺寸：**640 x 480**

## 运行控制

- **自动对焦**
  - 按下用户按键后触发 OV5640 自动对焦。
- **检测日志开关**
  - 在 `src/hal_entry.c` 中修改 `DETECT_RESULT_LOG_ENABLE`
  - `0`：关闭 `detect box num` / `Time elapsed` 日志
  - `1`：打开日志

## 说明

- 当前工程在人脸框显示时使用 framebuffer 叠加渲染路径。
- 当前 MIPI 摄像头链路中，OV5640 通过 **I2C0** 配置。

## RT-Thread / FSP 配置要点

工程需要启用以下模块：

- **MIPI CSI**
- **VIN**
- **Ethos-U55 / rm_ethosu**
- **I2C0 摄像头控制**
- **RGB565 LCD / GLCDC**

## 编译与下载

可通过 **RT-Thread Studio** 或仓库现有的工程构建流程完成编译，然后通过开发板调试接口下载固件。

## 运行效果

系统启动后：

- LCD 显示 OV5640 实时画面
- 检测到人脸时在屏幕上绘制绿色矩形框
- 按下用户按键可触发摄像头自动对焦


![alt text](figures/image-20251029173335454.png)
