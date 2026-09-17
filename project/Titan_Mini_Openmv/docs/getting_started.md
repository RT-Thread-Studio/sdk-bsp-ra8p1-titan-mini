# 上手指南

适用配置：RA8P1 Titan Mini、OV5640 MIPI 摄像头、SD 卡固件。Flash 配置见 [存储说明](storage.md)。

## 1. 准备硬件和文件

硬件：开发板、摄像头、USB 数据线、FAT / FAT32 格式 SD 卡，以及支持 RA8P1 的烧录器。电脑安装 OpenMV IDE 和烧录软件。接线与供电参见 [用户手册](../../../docs/Titan_Mini_user_manual.pdf) 和 [原理图](../../../docs/Titan_Mini_schematic_v1.0.pdf)。摄像头、SD 卡在断电状态下插拔。

| 文件 | 获取位置 | 用途 |
|---|---|---|
| `firmware/rtthread.hex` | [编译并收集固件](development.md) | 片内固件 |
| `firmware/openmv-vision.bin` | 与 HEX 一起生成 | 板端存储根目录的视觉资源 |
| 示例脚本 | [examples](../examples/README.md) | 在 IDE 中打开并运行 |
| 模型与标签 | [models/sdcard](../models/sdcard/) | 第 4 节使用的 `.tflite` 和 `.txt` |
| 图片等素材 | 各示例的 `assets/` 目录 | 复制位置见对应示例说明 |

以下路径相对于 `project/Titan_Mini_Openmv/`，工具命令在该目录执行。日常使用直接从 `examples/` 打开脚本、从 `models/` 复制模型；`firmware/` 仅包含上述两个固件文件。

## 2. 烧录与部署视觉资源

1. 连接烧录器，在烧录软件中选择 RA8P1 目标，将 **`firmware/rtthread.hex`** 下载到片内存储器。
2. 断电插入 SD 卡，连接 OpenMV USB 数据口并上电。电脑枚举 USB 串口，SD 卡就绪后显示存储卷。
3. 将配套的 **`firmware/openmv-vision.bin`** 复制到存储卷根目录，保持文件名和内容不变。读卡器复制使用相同路径。
4. 复制完成后，在操作系统中安全弹出存储卷。视觉加载成功时，UART1 输出 `[titan.vision] verified ...`。

HEX 与视觉 BIN 必须来自同一次构建。视觉 BIN 由固件解压并加载到 SDRAM；替换已加载的文件后，执行硬件复位。烧录使用 HEX，调试用 ELF 位于 `build/firmware/`。

## 3. 连接 IDE 并预览相机

1. 启动 OpenMV IDE，选择开发板 USB 端口并连接。IDE 的 **AE3** 板型名称为协议兼容标识；官方 Alif AE3 固件与 RA8P1 不兼容。
2. 通过“文件 → 打开”打开 [LED 示例](../examples/03_外设/01_GPIO与LED/blinky.py)，点击运行，确认板载 LED 闪烁。
3. 停止 LED 示例，打开 [相机预览](../examples/01_视觉/00_入门/helloworld.py) 并运行。图像为 QVGA（320×240）RGB565，终端持续输出循环帧率。
4. 调整镜头焦距。切换示例前停止当前脚本。

示例通过 IDE 打开本地文件运行，不依赖板型示例列表。无 SD 卡或视觉资源时，LED 和基础 Python 可运行；相机、图像和模型模块需要视觉资源。

## 4. 运行第一个 NPU 示例

### FOMO 人脸中心检测

脚本：[tf_object_detection.py](../examples/02_机器学习/00_神经网络/tf_object_detection.py)。

1. 从 `models/sdcard/` 复制以下两个文件到存储卷根目录：`fomo_face_detection_u55_256.tflite`、`fomo_face_detection_u55_256.txt`。
2. 安全弹出存储卷，在 IDE 中打开并运行 `tf_object_detection.py`。
3. 将人脸置于画面中，观察人脸中心标记。

模型的输入类型、归一化及后处理依赖见 [机器学习示例](../examples/02_机器学习/README.md)。Flash 固件使用相同模型，容量限制见 [存储说明](storage.md)。

### 单手关键点

[单手关键点示例](../examples/02_机器学习/00_神经网络/hand_landmarks_single_hand.py) 使用以下两个模型，保持原文件名复制到板端存储根目录：

| `models/sdcard/` 中的文件 | 示例加载路径 |
|---|---|
| `palm_detection_full_192_u55_256.tflite` | `/rom/palm_detection_full_192_u55_256.tflite` |
| `hand_landmarks_full_224_u55_256.tflite` | `/rom/hand_landmarks_full_224_u55_256.tflite` |

复制完成并安全弹出存储卷后运行。测试场景包括无手搜索、稳定跟踪、移出画面和重新进入。计时方法见 [性能与对比](performance.md)。

## 5. 素材与自启动

模型、图片和脚本分别传输。按 [示例目录](../examples/README.md) 和脚本对应说明，将所需素材复制到指定路径，保留 `omv_assets/...` 层级。`/rom` 为根目录只读别名：`/rom/model.tflite` 对应根目录的 `model.tflite`，无需建立 `rom` 文件夹。

按示例说明从对应 `assets/` 目录复制所需素材，并保留板端目标路径。固件收集不导出示例和模型包。

脱离 IDE 自启动：

1. 在 IDE 中运行应用，检查模型和素材加载。
2. 备份存储卷上已有的 `main.py`，将应用脚本另存为根目录的 `main.py`。它导入的其他 Python 文件放在根目录或 `lib/`。
3. 停止电脑文件访问并安全弹出，关闭 IDE 和占用该 USB 串口的终端，然后硬件复位。
4. 视觉资源就绪后，固件依次执行 `boot.py`、`main.py`。`main.py` 每次硬件启动自动执行一次；IDE Start/Stop 和 Ctrl-D 不触发再次自启动。

关闭 IDE 图像预览：

```python
import board
board.preview(False)
```

停用自启动：重命名或移走 `main.py`，然后硬件复位。IDE 执行/停止请求支持接管自启动等待和运行状态。

## 6. 常见问题

| 现象 | 检查方法 |
|---|---|
| USB 串口不存在 | 检查数据线、USB 数据接口和供电；硬件复位后重新枚举 |
| 串口存在但没有盘符 | 检查固件存储后端、SD 卡及 FAT 文件系统 |
| 导入 `csi` / `image` / `ml` 报资源错误 | 核对 `openmv-vision.bin` 在根目录、与 HEX 来自同一次构建；复制后安全弹出，更新后硬件复位 |
| 模型或素材找不到 | 核对脚本需要的完整文件名、标签和路径；`/rom` 不对应实体文件夹 |
| IDE 显示 AE3 或没有本板示例 | 使用本工程脚本和固件；直接在 IDE 打开本地示例 |
| 换卡后仍无介质 | 已枚举的 SD 拔出后需复位；先结束文件访问，再断电换卡 |
| 相机颜色异常或标签识别失败 | 先运行预览、调整焦距；保持示例默认方向设置，不直接改写 OV5640 翻转寄存器 |
| 帧率低于预期 | 对齐图像尺寸、模型版本、目标数、预览与打印开关；按性能文档分别测量 |

诊断接口见 [USB](usb.md)、[存储](storage.md) 和 [外设接口](peripherals.md)。
