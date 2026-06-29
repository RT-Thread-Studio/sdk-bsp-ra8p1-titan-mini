# WAV 音频播放器示例说明

[**中文**] | [**English**](./README.md)

## 简介

本示例展示了如何在 **Titan Board Mini** 上，使用 **ES8156 音频编解码器** 和 **SSI(I2S)** 接口播放 WAV 音频文件，并通过 **RT-Thread Audio Framework** 完成音频输出。

本工程当前仅支持以下音频格式：

- **WAV (RIFF)**
- **PCM 编码**
- **16 种采样率**
- **16-bit 位深**
- **单声道 / 双声道**

不满足以上条件的音频文件将无法正常播放。

## 主要功能

- 基于 ES8156 的 WAV 音频播放
- 通过 SSI 接口输出 I2S 音频数据
- 支持 `wavplay` 命令播放本地文件
- 支持音量调节、暂停、恢复、停止
- 支持从 SD 卡文件系统读取音频文件

## 硬件说明

### 1. ES8156 音频编解码器

**Titan Board Mini** 板载 **ES8156** 音频编解码器，主要参数如下：

| 参数 | 说明 |
|------|------|
| 型号 | ES8156 |
| 类型 | 立体声 DAC |
| 分辨率 | 24-bit |
| 采样率范围 | 8kHz - 192kHz |
| 音频接口 | I2S / PCM |
| 控制接口 | I2C |

### 2. SSI 音频接口

RA8P1 的 SSI 接口用于输出 I2S 音频数据：

- 支持标准 I2S 协议
- 支持 DMA 传输
- 本工程支持 `16 种采样率 / 16-bit / 单双通道 PCM WAV`

## 工程结构

```text
Titan_Mini_wavplayer/
├── src/
│   └── hal_entry.c
├── libraries/
├── packages/
│   └── wavplayer-latest/
└── README_zh.md
```

## 使用说明

### 1. 启动提示

系统启动后会在串口输出当前工程说明，并提示当前支持：

- `16 种采样率`
- `16-bit`
- `mono / stereo PCM WAV`

### 2. 播放命令

将符合要求的 WAV 文件放入 SD 卡后，使用 msh 命令播放：

```bash
msh > wavplay -s /sdcard/test.wav
```

### 3. 音频文件格式要求

播放文件必须满足以下全部条件：

- **格式**：标准 WAV（RIFF）
- **编码**：PCM
- **采样率**：16 种受支持采样率之一
- **位深**：16-bit
- **声道**：单声道或双声道
- **扩展名**：`.wav`

## 运行说明

请先将音频文件转换为 **PCM、16-bit、单声道或双声道** 的 WAV 格式，并确保采样率属于支持的 16 种之一，再拷贝到 SD 卡中。开发板上电并挂载 SD 卡后，执行 `wavplay -s 文件名` 即可播放。

![alt text](figures/image1.png)

## 相关资料

- [RT-Thread 音频框架文档](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/audio/audio)
