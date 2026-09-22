# 性能对比

对比平台：RA8P1 Titan Mini、OpenMV Cam H7 Plus、OpenMV Cam RT1062、OpenMV N6、OpenMV AE3。

## 平台配置

| 项目 | RA8P1 Titan Mini | OpenMV H7 Plus | OpenMV RT1062 |
|---|---|---|---|
| CPU | Cortex-M85，1 GHz | Cortex-M7，480 MHz | Cortex-M7，600 MHz |
| CPU 计算扩展 | DSP、FPU、Helium MVE | DSP、FPU | DSP、FPU |
| NPU | Ethos-U55-256，500 MHz | 无 | 无 |
| NPU INT8 理论峰值 | 256 GOPS | — | — |
| 外部 SDRAM | 32 MiB，16-bit | 32 MB，32-bit，100 MHz | 32 MB，16-bit，160 MHz |
| 相机接口 | OV5640，MIPI-CSI / VIN | OV5640，并行 DCMI | OV5640，并行 CSI |
| JPEG 编码 | 软件 | 硬件 | 软件 |
| USB | HS，480 Mb/s | FS，12 Mb/s | HS，480 Mb/s |

规格来源：[Renesas RA8P1](https://www.renesas.com/en/products/ra8p1)、[H7 Plus](https://openmv.io/products/openmv-cam-h7-plus)、[RT1062](https://openmv.io/products/openmv-cam-rt)。

## CPU 跑分

| 平台 | CPU 主频 | 单核 CoreMark | 数据来源 |
|---|---:|---:|---|
| RA8P1 Cortex-M85 | 1 GHz | 6362 | [EEMBC 提交结果](https://www.eembc.org/viewer/?benchmark_seq=13714) |
| OpenMV H7 Plus | 480 MHz | 2400 | OpenMV 官方规格 |
| OpenMV RT1062 | 600 MHz | 3020 | OpenMV 官方规格 |

## 图像处理耗时

分辨率：**QVGA（320×240）**。单位：**ms / 次，平均值，越小越快**。

RA8P1：CPU 1 GHz，I/D cache 开启；每项运行 10 次，3 轮各采样 100 次。分别使用 SDRAM、SRAM 工作图，计时不含采集、输入恢复和预览。

| 测试项目 | 格式 / 参数 | H7 Plus（官方） | RT1062（官方） | RA8P1 SDRAM（实测） | RA8P1 SRAM（实测） |
|---|---|---:|---:|---:|---:|
| 反色 `invert()` | 灰度 | 0.80 | 0.74 | 0.937 | 0.085 |
| 帧差 `difference()` | 灰度 | 0.87 | 1.13 | 2.065 | 1.218 |
| 腐蚀 / 膨胀 | 灰度，3×3 | 4.27（官方共列） | 3.88（官方共列） | 2.735 / 2.745 | 2.134 / 2.138 |
| 均值滤波 `mean()` | 灰度，3×3 | 4.36 | 4.03 | 4.103 | 3.968 |
| 卷积 `morph()` | 灰度，3×3 | 11.87 | 10.05 | 2.915 | 2.762 |
| JPEG 编码 | 灰度，quality=90 | 1.25 | 5.54 | 4.140 | 3.479 |
| JPEG 编码 | RGB565，quality=90 | 4.94 | 16.44 | 10.517 | 9.575 |
| 灰度转换 `to_grayscale()` | RGB565 | 4.60 | 4.69 | 3.863 | 2.348 |

RA8P1 使用本地采集图；帧差参考为输入的反色图，卷积使用 3×3 锐化核。两块 OpenMV 的数值取自官方公开测试。

数据来源：[OpenMV 官方基准](https://docs.google.com/spreadsheets/d/1-FNVKCEr8-6UYs8MUm6wgsOt2c8ihJ2mg9QXKkG91os/edit?usp=sharing)及 2026-09-17 的 RA8P1 板端实测。

## RA8P1 / OpenMV N6 / AE3 NPU 推理对比

单位 **ms / 次，越小越快**。RA8P1 为本板实测，N6 / AE3 为 OpenMV 官方公布数据。

| 平台 | CPU | NPU |
|---|---|---|
| RA8P1 Titan Mini | Cortex-M85，1 GHz | Ethos-U55-256，500 MHz |
| OpenMV N6 | Cortex-M55，800 MHz | Neural-ART，1 GHz |
| OpenMV AE3 | Cortex-M55，400 MHz | Ethos-U55，400 MHz |

| 模型 | 输入尺寸 | RA8P1 / ms | OpenMV N6 / ms | OpenMV AE3 / ms |
|---|---|---:|---:|---:|
| YOLOv8n COCO-person（ST） | 192×192 | 28.53 | 24.15 | 51.47 |
| YOLOv8n COCO-person（ST） | 256×256 | 38.33 | 33.50 | 75.49 |
| YOLOv8n COCO-person（ST） | 320×320 | 287.49 | 40.50 | 107.26 |
| Tiny YOLOv2 INT8（ST） | 224×224 | 50.31 | 36.33 | 75.90 |
| Tiny YOLOv2 INT8（ST） | 416×416 | 58.49 | 42.36 | 117.10 |

RA8P1：Vela 5.0.0、COP1，每项 90 次预测调用均值，不含相机与后处理。本组模型的权重与命令流位于外部 SDRAM。YOLOv8n 192、256 和两个 Tiny YOLOv2 模型的工作区位于片内 SRAM，其余工作区位于 16-bit SDRAM。Tiny YOLOv2 416 使用尺寸优先配置。

官方模型 ID：`yolov8n_quant_pc_uf_od_coco-person-st`、`tiny_yolo_v2_int8-st`。编译选项为 `max pref`；AE3 的 Tiny YOLOv2 416×416 使用 `min size`。

数据来源：[OpenMV 官方 YOLO Models 测试表](https://docs.google.com/spreadsheets/d/1-FNVKCEr8-6UYs8MUm6wgsOt2c8ihJ2mg9QXKkG91os/edit#gid=1692885012)及 2026-09-22 的 RA8P1 五模型实测。

当前随源码提供的模型及部署方法见 [模型说明](../models/README.md)。

## 神经网络示例模型推理耗时

测试范围：[神经网络示例](../examples/02_机器学习/00_神经网络)，23 个脚本对应 20 个 NPU 模型和 1 个 CPU 回归模型。单位 **ms / 次**。

RA8P1 测试固件：HEX SHA-256 前缀 `5aace4b1`。使用示例默认模型，固定输入，预热 5 次、3 轮各采样 30 次；计时包含输入复制、推理和输出回调，不含相机采集、图像预处理、检测后处理与绘图。

| 模型 | 输入 | RA8P1 实测 / ms | OpenMV N6 / ms | OpenMV AE3 / ms |
|---|---|---:|---:|---:|
| FOMO 人脸中心检测 | 96×96 | 1.086 | — | — |
| BlazeFace（FLOAT32 接口） | 128×128 | 14.988 | — | — |
| BlazeFace（Renesas INT8） | 128×128 | 13.061 | — | — |
| BlazePalm 手掌检测 | 192×192 | 57.683 | — | — |
| Face Landmarks 468 点 | 192×192 | 16.839 | — | — |
| Hand Landmarks 21 点 | 224×224 | 31.586 | — | — |
| MoveNet 单人姿态 | 192×192 | 22.918 | — | — |
| YOLO LC 行人检测 | 192×192 | 5.941 | — | — |
| YOLOv8n person（示例 Vela 4） | 192×192 | 36.685 | 24.15 | 51.47 |
| YOLOv8n COCO80 | 192×192 | 29.787 | — | — |
| YOLOv8n Pose | 192×192 | 28.294 | — | — |
| YOLOv5n6-xiao VOC20 | 192×192 | 6.849 | — | — |
| YOLO-Fastest 灰度人脸 | 192×192 | 5.789 | — | — |
| Swift-YOLO Gesture | 192×192 | 8.223 | — | — |
| Swift-YOLO Pet | 192×192 | 7.916 | — | — |
| Swift-YOLO Nano Person | 192×192 | 14.940 | — | — |
| U-Net MobileNetV2 分割 | 128×128 | 46.091 | — | — |
| VWW 人体存在分类 | 96×96 | 0.609 | — | — |
| ResNet8 CIFAR10 | 32×32 | 2.261 | — | — |
| MobileNet V1 0.25 | 224×224 | 5.370 | — | — |
| 数值回归（CPU） | 1×36 | 0.051 | — | — |

单脸/多脸共用 BlazeFace 与 Face Landmarks；单手/多手共用 BlazePalm 与 Hand Landmarks。表中为各模型独立调用耗时。YOLOv8n person 使用示例自带的 Vela 4 CPU Softmax 版本。

“—”表示未检索到对应模型的 N6 / AE3 公开耗时。YOLOv8n 数据来自 [OpenMV 官方测试表](https://docs.google.com/spreadsheets/d/1-FNVKCEr8-6UYs8MUm6wgsOt2c8ihJ2mg9QXKkG91os/edit#gid=1692885012)。RA8P1 数据为 2026-09-17 的示例模型测试结果。

### 单手跟踪示例帧率

示例：[hand_landmarks_single_hand.py](../examples/02_机器学习/00_神经网络/hand_landmarks_single_hand.py)，使用 BlazePalm 192×192 和 Hand Landmarks 224×224。

| 测试项目 | RA8P1 实测 / FPS | OpenMV N6 / FPS | OpenMV AE3 / FPS |
|---|---:|---:|---:|
| 单手检测与关键点跟踪 | 21.88 | 约 22 | 约 17 |

RA8P1：RGB565、400×400 画面，计时包含采集、预处理、推理、后处理和绘图，关闭 IDE 预览传输。30.022 秒处理 657 帧，653 帧检测到手部。

数据来源：2026-09-17 的 RA8P1 单手跟踪实测及 [Derek Molloy 的 N6 / AE3 实测](https://derekmolloy.ie/the-openmv-ae3-a-time-of-flight-doorman-for-the-npu/)（2026-07-26）。

### AE3 SDK 公开参考数据

| 模型 | 输入 | AE3 / ms | 来源 |
|---|---|---:|---|
| BlazeFace Front | 128×128 | 约 14 | Tendrl SDK |
| BlazePalm Full | 192×192 | 约 44 | Tendrl SDK |

来源：[Tendrl SDK Vision 文档](https://tendrl.com/docs/contact/sdks/micropython/vision/)，原文指标为 Inference，未细分预处理与后处理。

## RA8P1 CPU / NPU 推理对比

平台：**Renesas EK-RA8P1 官方参考工程**；CPU 1 GHz、NPU 500 MHz、D-cache 开启，RUHMI / FSP 6.4.0。

| 模型 | 输入 | CPU / ms | NPU / ms |
|---|---|---:|---:|
| MobileNet V1 0.25 INT8 | 224×224×3 | 98 | 1–2 |
| EfficientNet Lite0 INT8 | 224×224×3 | 1553 | 137 |

数据来源：[Renesas 应用笔记，Rev.1.20，表13/14](https://www.renesas.com/en/document/apn/building-vision-ai-application-using-ra8p1-mcu-ethos-u55-npu)。

## Titan Mini 模型实测

配置：RA8P1 Titan Mini、OV5640、SD 存储，固件版本 5.0.1。模型输入为 192×192，相机输出为 QVGA。

| 模型 | 推理调用均值 / ms | 最小–最大 / ms | 相机应用帧率 / FPS |
|---|---:|---:|---:|
| Swift-YOLO Gesture | 8.25 | 8.21–8.36 | 41.22 |
| Swift-YOLO Pet | 7.94 | 7.90–8.05 | 41.22 |
| Swift-YOLO Nano Person | 14.97 | 14.93–15.07 | 41.21 |
| YOLO-Fastest Face | 5.79 | 5.76–5.89 | 41.22 |
| YOLOv5n6-xiao VOC20 | 6.90 | 6.84–7.03 | 41.22 |

推理调用为固定输入的 12 次测量均值，包含输入复制及 TFLM 调用；应用帧率为 30 秒采集、推理、后处理、绘图及预览测试结果。
