# Titan Mini OpenMV

RA8P1 Titan Mini 的 OpenMV 移植工程。运行环境为 RT-Thread 5.1、OpenMV 5.0.1 和 MicroPython，神经网络后端为 TensorFlow Lite Micro + Ethos-U55。

## 硬件与功能

| 项目 | 配置 |
|---|---|
| 处理器 | Cortex-M85，1 GHz |
| 神经网络加速器 | Ethos-U55-256，500 MHz |
| 相机 | OV5640，双 lane MIPI-CSI / VIN |
| 采集格式 | RGB565、GRAYSCALE；QQVGA、QVGA、VGA |
| USB | USB 2.0 HS，CDC + MSC；CDC 连接 OpenMV IDE，MSC 传输文件 |
| 文件系统 | FAT；默认 SDHI0，支持编译选择 W25Q64 的 6 MiB 分区 |
| Python 外设 | GPIO、IRQ、PWM、ADC、UART、I2C、SPI、软件定时器 |
| 图像处理 | 色块、巡线、条码、AprilTag、特征、滤波、几何变换 |
| 神经网络 | 分类、目标检测、姿态与手脸关键点；模型清单见 `models/MODEL_MANIFEST.json` |

IDE 板型标识为 AE3，固件和引脚定义均采用本工程的 RA8P1 配置。官方 Alif AE3 固件与本板不兼容。原生 JPEG / Bayer 采集、自动对焦、网络、蓝牙、display 和音频未启用。

## 快速使用

1. 准备 FAT / FAT32 格式的 SD 卡，连接 OV5640 摄像头。
2. 烧录 `firmware/rtthread.hex`，将配套 `firmware/openmv-vision.bin` 复制到 SD 卡根目录。两个文件必须来自同一次构建。
3. 安全弹出存储卷，连接 OpenMV IDE，运行 [相机预览](examples/01_视觉/00_入门/helloworld.py)。
4. 从 [机器学习示例](examples/02_机器学习/README.md) 选择应用，将对应模型和标签从 `models/sdcard/` 复制到存储根目录，再运行脚本。

接线、烧录和自启动操作见 [上手指南](docs/getting_started.md)。`firmware/` 仅存放 HEX 和视觉 BIN；从源码生成这两个文件的步骤见 [编译与验证](docs/development.md)。

配置好工具链后，在工程目录执行 `scons -j8` 即可编译并自动生成 `firmware/` 中的发布文件，无需手工运行额外的生成或打包脚本。

也可从 GitHub Actions 的 **OpenMV Firmware** 工作流下载 SD / Flash 配置的预编译固件，操作见 [CI 固件下载](docs/development.md#github-ci-与固件下载)。

## 文档

| 文档 | 内容 |
|---|---|
| [上手指南](docs/getting_started.md) | 固件烧录、IDE、相机、模型与自启动 |
| [性能对比](docs/performance.md) | 平台配置、图像处理耗时与模型推理数据 |
| [外设接口](docs/peripherals.md) | 引脚、API 与共享资源 |
| [存储](docs/storage.md) | 存储配置、FAT、MSC 与路径映射 |
| [USB](docs/usb.md) | 协议、连接状态与故障诊断 |
| [架构](docs/architecture.md) | 任务、内存、采集及推理数据流 |
| [编译与验证](docs/development.md) | 工具链、SCons、发布校验与产物 |
| [Python 示例](examples/README.md) | 分类示例及资源依赖 |

组件版本见 [ThirdParty](ThirdParty/README.md)，模型来源和许可证见 [models](models/README.md)。
