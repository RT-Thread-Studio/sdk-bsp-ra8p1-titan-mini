# Python 示例

适用硬件：RA8P1 Titan Mini、OV5640。示例对应 OpenMV v5.0.1，共 194 个独立脚本。

| 分类 | 脚本数 | 内容 |
|---|---:|---|
| [01_视觉](01_视觉/README.md) | 124 | 相机、滤波、色块、光流、条码、特征与 AprilTag |
| [02_机器学习](02_机器学习/README.md) | 28 | 分类、检测、姿态、手脸关键点、Haar 与数值回归 |
| [03_外设](03_外设/README.md) | 24 | GPIO、IRQ、PWM、ADC、UART、I2C、SPI |
| [04_系统与存储](04_系统与存储/README.md) | 11 | Timer、asyncio、系统信息与文件读写 |
| [05_通信](05_通信/README.md) | 5 | Arduino UART、Pixy 与 MAVLink 数据输出 |
| [06_性能测试](06_性能测试/README.md) | 2 | 图像算法计时 |

## 运行步骤

1. 烧录 `rtthread.hex`，将同次编译生成的 `openmv-vision.bin` 复制到开发板存储根目录。
2. 在 OpenMV IDE 连接开发板，先运行 [LED 示例](03_外设/01_GPIO与LED/blinky.py)，再运行 [相机预览](01_视觉/00_入门/helloworld.py)。
3. 按脚本开头的说明连接外设、复制资源并设置参数，然后运行该脚本。每次运行一个示例，切换前点击停止；切换模型前软复位。

## 资源位置

| 资源 | 位置与复制方式 |
|---|---|
| 视觉资源 | `openmv-vision.bin` 放在开发板存储根目录 |
| 图像、Haar 文件和辅助脚本 | 源码中的 `01_视觉/assets/`、`02_机器学习/assets/`；按示例开头指定的路径复制 |
| 神经网络模型 | 见 [机器学习示例](02_机器学习/README.md)，模型与同名标签文件一起复制 |
| AT&T 人脸数据集 | 按 `face_recognition.py` 的说明准备 `orl_faces/` |

脚本中的 `/rom/` 是开发板存储根目录的只读别名，不需要建立 `rom` 文件夹。在 IDE 中运行电脑上的脚本时，模型和素材仍从开发板存储读取。复制完成后安全弹出磁盘，再运行需要读写文件的脚本。

模型和标签位于源码工程 `models/sdcard/`。按 [机器学习示例](02_机器学习/README.md) 的模型表选择文件，保持原文件名复制到板端存储根目录；Hand 示例也直接使用带 `_u55_256` 后缀的模型文件名。

## 外设与许可

相机支持 QQVGA、QVGA、VGA 的 RGB565 和 GRAYSCALE。用户串口为 UART2，UART1 用于调试控制台；接线见 [BOARD.md](BOARD.md)。

脚本许可见 [LICENSE](LICENSE)。模型来源及许可见 [模型说明](../models/README.md) 和 [模型许可目录](../models/licenses/)，其他素材的归属声明保留在对应 `assets/` 目录。
