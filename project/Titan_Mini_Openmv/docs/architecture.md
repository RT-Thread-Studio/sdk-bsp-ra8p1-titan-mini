# 软件架构

## 运行时组成

| 层次 | 实现 | 目录 |
|---|---|---|
| 应用 | MicroPython 脚本 | `examples/` |
| Python 运行时 | OpenMV 模块、MicroPython VM / GC / VFS | `platform/micropython/`、`platform/openmv/` |
| 视觉处理 | imlib、AprilTag、CMSIS-DSP | `ThirdParty/openmv/`、`ThirdParty/apriltag/` |
| 推理 | TFLM、CMSIS-NN、Ethos-U core driver | `platform/tflm/`、`platform/npu/` |
| 板级驱动 | MIPI / VIN、USB、SD / Flash、machine | `platform/camera/`、`usb/`、`storage/`、`machine/` |
| 系统 | RT-Thread、Renesas FSP、SDK HAL | SDK 的 `rt-thread/`、`FSPConfiguration/ra/`、`libraries/` |

组件版本见 [ThirdParty](../ThirdParty/README.md)。

## 任务与启动顺序

| 任务 | RT-Thread 优先级 | 工作 |
|---|---:|---|
| USB | 15 | TinyUSB 事件、CDC / MSC 协议 |
| MicroPython VM | 20 | 脚本执行、IDE 协议、REPL、GC |
| MSC worker | 25 | 块设备 I/O、介质刷新 |
| 资源加载 | 25 | FAT 挂载、视觉文件加载 |

系统先启动 USB 与 VM，再由后台任务挂载介质、加载视觉资源。资源校验完成后，VM 执行 `boot.py` 和 `main.py`。无介质或视觉文件缺失时，CDC、REPL 和基础 Python 模块仍运行。

MSC 块设备 I/O 在独立任务中执行。IDE 的执行/停止请求在自启动等待和脚本运行期间有效。

## 内存布局

| 区域 | 配置容量 | 分配内容 |
|---|---:|---|
| 片内 MRAM | 1 MiB | VM、协议栈、驱动、视觉加载器 |
| SRAM | 链接区 1872 KiB | 静态数据、1344 KiB 主 GC 池 |
| ITCM | 128 KiB | VM、图像和 CMSIS-NN 热点代码 |
| DTCM | 128 KiB | 32 KiB VM 栈及 CPU 临时数据 |
| 外部 SDRAM | 32 MiB | 图像池、模型、扩展 GC、RT 堆、视觉段 |
| SDRAM 视觉段 | `0x68C00000`，4 MiB | 图像算法、常量、AprilTag 字典 |

SDRAM 包含 8 MiB 图像池和 10 MiB 扩展 GC，布局见 [ra8_memory.h](../platform/include/ra8_memory.h)。`board.memory()` 返回运行时内存状态；

DTCM 不在 NPU / 外设 DMA 的可访问区域内。模型、命令流和 tensor arena 分配到 DMA 可访问的 SRAM 或 SDRAM。

## 视觉代码加载

视觉算法由 `firmware/openmv-vision.bin` 提供。启动时从板端存储根目录读取，解压到 SDRAM，经固件绑定和 CRC 校验后开放图像、相机和推理模块。

视觉文件与 `firmware/rtthread.hex` 来自同一次构建。文件缺失或损坏时，USB 和基础 Python 保持运行，视觉模块返回资源错误。替换已加载的视觉文件后需硬件复位。

## 图像数据流

```text
OV5640 → MIPI-CSI → VIN DMA → OpenMV framebuffer
                              ↓
                    步长/灰度/方向处理
                              ↓
                   image 算法 / ml.Model
                              ↓
                    绘图 → JPEG → USB CDC
```

VIN 直接写入 OpenMV 帧缓冲。采集完成后处理灰度、行步长及图像方向，再交给 Python。`vflip` / `hmirror` 通过像素变换实现，应用无需修改 OV5640 方向寄存器。

`board.preview(False)` 关闭 JPEG 预览传输。`board.pipeline(False)` 将采集设为单次模式，`snapshot()` 返回前停止采集；默认流水采集模式用于连续视觉应用。

## 推理数据流

```text
image / 输入回调 → 缩放、量化 → TFLM
                              ├─ builtin 算子 → CMSIS-NN / reference CPU kernel
                              └─ ethos-u 算子 → FSP → Ethos-U55
                                          ↓
                              输出复制 → 后处理
```

CPU 为 Cortex-M85 1 GHz，NPU 为 Ethos-U55-256 500 MHz。Vela 编译时将可加速部分替换为 `ethos-u` 子图，TFLM 调度其余 CPU 节点。传统图像算法和 Python 应用逻辑在 CPU 上执行。

大模型文件分配到 SDRAM，推理工作区（arena）优先分配到 SRAM，空间不足时回退到 SDRAM。`print(model)` 的 `ram_addr` 和 `ram_size` 给出工作区地址和需求：`0x220...` 对应 SRAM，`0x68...` / `0x69...` 对应 SDRAM。

随工程提供的 Palm / Hand 模型配合标准后处理，自动共用 1184 KiB 工作区。两个模型顺序执行；显式传入 `workspace=None` 使用私有工作区。模型及输入输出要求见 [机器学习示例](../examples/02_机器学习/README.md)。

构建、测试和维护入口见 [编译与验证](development.md)。
