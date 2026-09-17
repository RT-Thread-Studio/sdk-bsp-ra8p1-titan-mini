# USB 接口与故障诊断

USB 使用 TinyUSB CDC + MSC 复合设备，VID:PID 为 `37C5:16E3`。

| 接口 | 功能 |
|---|---|
| CDC | OpenMV IDE 协议、Python REPL |
| MSC | 导出编译配置所选的 SD / SPI Flash 存储卷 |

IDE 中的 AE3 名称为协议兼容标识，硬件和固件目标为 RA8P1。Windows 重新枚举后可能分配新的 COM 号。

## 启动状态

| 条件 | CDC / 基础 Python | MSC | 视觉模块 |
|---|---|---|---|
| 介质和视觉文件正常 | 可用 | 导出存储卷 | 加载完成后可用 |
| 无卡或介质初始化未完成 | 可用 | 无介质 | 未就绪 |
| 视觉文件缺失、损坏或版本不匹配 | 可用 | 介质正常时可用 | 导入 `image`、`csi`、`ml` 等返回资源错误，预览为空 |

MSC 在介质就绪后自动导出。复制文件前停止板端写入，复制完成后安全弹出，再运行应用。操作步骤见 [上手指南](getting_started.md) 和 [存储说明](storage.md)。

## 介质导出接口

`board.usb_msc()` 返回当前导出状态。带布尔参数时修改状态，并返回修改后的状态：

```python
import board
print(board.usb_msc())  # 查询介质是否向电脑提供
board.usb_msc(False)    # 隐藏介质，板端文件系统仍可访问
board.usb_msc(True)     # 重新提供介质
```

隐藏介质前，先完成主机读写并执行操作系统安全弹出。MSC 弹出后 CDC 保持连接，板端 FAT 保持挂载。UART1 `msh` 提供对应命令：`usb_msc on`、`usb_msc off`、`usb_msc status`。

## 视觉资源加载与更新

`openmv-vision.bin` 与 HEX 中的视觉内容标识绑定。后台线程读取文件并解压到 SDRAM，校验文件头、大小、CRC 和绑定值，通过后开放视觉模块。

1. 烧录 `firmware/rtthread.hex`，将配套 `firmware/openmv-vision.bin` 复制到存储根目录。
2. 首次补齐缺失文件后，等待后台加载完成，再运行视觉脚本。
3. 替换已加载的视觉文件后，执行硬件复位。IDE Start/Stop 和 Ctrl-D 保留已加载资源。

UART1 加载成功日志：`[titan.vision] verified ...`。文件损坏或版本不匹配时，使用 MSC 替换文件后复位。`boot.py` / `main.py` 等待资源就绪；等待期间响应 IDE 执行/停止请求和串口 REPL。

## 终端

| 终端 | 用途 | 示例 |
|---|---|---|
| OpenMV USB CDC | IDE 或 Python REPL，单客户端占用 | `import board; print(board.memory())` |
| UART1 RT-Thread msh | 设备与存储管理，独立于 USB | `list device`、`mount`、`usb_msc status` |

USB 串口 REPL 连接步骤：关闭 IDE，停止自启动应用，以 115200 波特率、DTR 开启打开开发板端口。停用自启动时，重命名或移走 `main.py` 后硬件复位。UART1 接线见 [外设接口](peripherals.md)。

## 故障诊断

| 问题 | 检查顺序 |
|---|---|
| USB 无法枚举 | 数据线和接口 → 供电 → 硬件复位 → UART 启动日志 |
| CDC 正常而 MSC 无介质 | 存储配置 → `list device` → `mount` → `storage_mount`；构建后端见 `build/reports/manifest.json` |
| 复制后板端看不到文件 | 确认主机复制完成并安全弹出 → 关闭板端打开的文件 / 目录 → 复位后重新读取 |
| 视觉模块资源错误 | 文件位于根目录 → 与 HEX 同一次构建 → 重新复制配套视觉 BIN 并复位 |
| IDE 连接被占用 | 关闭串口终端及其他 IDE 实例，再重新选择端口 |
| 连续打印出现日志缺失 | 降低日志频率；stdout 缓冲有限，电脑读取过慢会丢弃旧日志 |
| NPU 不执行或报错 | 检查所用模型及输入 / 后处理 → 查看 IDE 错误输出 → 硬件复位后重试单个示例 |

实现与时序见 [架构](architecture.md)，构建与测试见 [编译与验证](development.md)。
