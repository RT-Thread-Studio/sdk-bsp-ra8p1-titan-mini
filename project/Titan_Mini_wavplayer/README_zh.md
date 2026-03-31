# WAV 音频播放器使用说明

**中文** | [**English**](./README.md)

## 简介

本示例展示了如何在 **Titan Board Mini** 上使用 **ES8156 音频编解码器** 实现 **WAV 格式音频播放**,通过 **SSI (Synchronous Serial Interface)** 接口传输音频数据,结合 **RT-Thread 音频框架** 实现完整的音频播放功能。

主要功能包括：

- 使用 ES8156 编解码器实现高质量音频播放
- 支持 WAV 格式音频文件播放
- 通过 SSI 接口进行 I2S 音频数据传输
- 支持音量调节、暂停/恢复/停止等播放控制
- 支持从 SD 卡或 Flash 文件系统读取音频文件

## 硬件介绍

### 1. ES8156 音频编解码器

**Titan Board Mini** 板载 **ES8156** 高性能音频编解码器：

| 参数 | 说明 |
|------|------|
| **型号** | ES8156 |
| **制造商** | Everest Semiconductor (亿能电子) |
| **类型** | 立体声 DAC (数模转换器) |
| **分辨率** | 24-bit |
| **采样率** | 8kHz - 192kHz |
| **信噪比** | > 100dB |
| **输出功率** | 30mW + 30mW @ 16Ω |
| **接口** | I2S / PCM |
| **控制接口** | I2C |
| **工作电压** | 1.8V - 3.6V |

### 2. ES8156 主要特性

#### 音频性能

- **高保真音频**：24-bit DAC,支持高分辨率音频
- **宽采样率范围**：8kHz - 192kHz,覆盖各种音频场景
- **低噪声**：> 100dB 信噪比,提供清晰音质
- **低失真**：THD+N < -85dB
- **多格式支持**：I2S、Left-justified、Right-justified、PCM

#### 输出特性

- **立体声输出**：左右声道独立输出
- **耳机驱动**：内置耳机放大器,直接驱动 16Ω - 32Ω 耳机
- **线路输出**：支持线路电平输出
- **音量控制**：数字音量控制,范围 0-255,±0.5dB/Step

### 3. SSI (Synchronous Serial Interface) 接口

RA8P1 的 SSI 接口用于音频数据传输：

- **I2S 协议**：标准 I2S 音频协议
- **主从模式**：可配置为主机或从机模式
- **DMA 支持**：支持 DMA 传输,减少 CPU 占用
- **多通道**：支持单声道/立体声

## 软件架构

### 1. 分层设计

音频播放系统采用分层架构：

```
应用程序层 (用户代码)
    ↓
WAV Player Package - WAV播放器
    ↓
Audio Device Framework - RT-Thread 音频设备框架
    ↓
ES8156 Driver - ES8156驱动
    ↓
SSI/I2S Driver - SSI/I2S驱动
    ↓
FSP SSI HAL - 硬件抽象层
```

### 2. 核心组件

#### WAV Player Package

RT-Thread 的 WAV 播放器软件包,提供完整的音频播放功能：

- **WAV 文件解析**：解析 WAV 文件头,提取音频参数
- **播放控制**：播放、暂停、恢复、停止
- **音量控制**：0-99 级音量调节
- **状态管理**：播放状态机管理
- **多文件支持**：支持从不同存储介质播放

**主要接口** (`wavplayer.h`)：

```c
/* 播放 WAV 文件 */
int wavplayer_play(char *uri);

/* 停止播放 */
int wavplayer_stop(void);

/* 暂停播放 */
int wavplayer_pause(void);

/* 恢复播放 */
int wavplayer_resume(void);

/* 设置音量 (0-99) */
int wavplayer_volume_set(int volume);

/* 获取音量 */
int wavplayer_volume_get(void);

/* 获取播放状态 */
int wavplayer_state_get(void);
```

#### ES8156 驱动

ES8156 驱动提供编解码器的控制接口：

```c
/* 初始化 ES8156 */
rt_err_t es8156_device_init(void);

/* 获取设备句柄 */
struct es8156_device *es8156_get_device(void);

/* 设置音量 (0-255) */
void es8156_set_volume(rt_uint8_t volume);

/* 静音/取消静音 */
void es8156_mute(rt_bool_t mute);

/* 掉电模式 */
void es8156_powerdown(void);
```

**驱动配置**：
- **I2C 接口**：I2C1 (用于寄存器配置)
- **I2C 地址**：0x08 (ADDR引脚接GND)
- **模拟电压**：3.3V (可配置 1.8V/3.3V)
- **默认音量**：191 (0dB)

### 3. 工程结构

```
Titan_Mini_wavplayer/
├── src/
│   └── hal_entry.c          # 主程序入口
├── libraries/
│   └── HAL_Drivers/ports/
│       └── es8156/
│           ├── es8156.c     # ES8156 驱动实现
│           └── es8156.h     # ES8156 驱动头文件
└── packages/
    └── wavplayer-latest/    # WAV 播放器软件包
        ├── inc/
        │   ├── wavplayer.h  # 播放器接口
        │   ├── wavrecorder.h # 录音器接口
        │   └── wavhdr.h     # WAV 文件头定义
        └── src/
            ├── wavplayer.c      # 播放器实现
            ├── wavplayer_cmd.c  # 播放器命令
            ├── wavhdr.c         # WAV 文件解析
            └── wavrecorder.c    # 录音器实现
```

## 使用说明

### 1. 初始化流程

系统初始化时需要初始化 ES8156 和音频设备：

```c
#include <rtthread.h>
#include "es8156.h"

void hal_entry(void)
{
    /* 初始化 ES8156 编解码器 */
    if (es8156_device_init() != RT_EOK)
    {
        rt_kprintf("ES8156 initialization failed!\n");
        return;
    }

    rt_kprintf("ES8156 audio codec initialized successfully!\n");
    rt_kprintf("WAV Player is ready!\n");

    /* 主循环 */
    while (1)
    {
        /* 应用逻辑 */
        rt_thread_mdelay(1000);
    }
}
```

### 2. 播放 WAV 文件

使用 msh 命令行播放音频文件：

```bash
/* 播放 SD 卡中的 WAV 文件 */
msh >wavplay -s /sdcard/test.wav
```

### 3. WAV 文件格式要求

WAV 文件需要满足以下要求：

- **格式**：标准 WAV (RIFF) 格式
- **编码**：PCM 编码
- **采样率**：16kHz
- **位深**：16-bit
- **通道**：单声道或立体声
- **文件扩展名**：.wav

## 配置说明

### 1. Kconfig 配置

在 `libraries/M85_Config/Kconfig` 中配置音频选项：

```kconfig
menuconfig BSP_USING_AUDIO
    bool "Enable Audio"
    select BSP_USING_SSI0
    select BSP_USING_ES8156
    default n
    if BSP_USING_AUDIO
        config BSP_USING_AUDIO_VOLUME
            int "Default volume (0-99)"
            default 50
    endif
```

### 2. RT-Thread Settings

在 RT-Thread Studio 中,需要启用以下组件：

1. **设备驱动**
   - 启用 SSI/I2C 设备驱动
   - 启用 Audio 设备

2. **软件包**
   - 添加 wavplayer-latest 软件包

3. **组件**
   - 启用 DFS 文件系统
   - 启用 SD 卡的文件系统

### 3. 硬件连接

ES8156 与 Titan Board Mini 的连接：

- **I2C 控制接口**：
  - I2C1_SDA / I2C1_SCL
  - 用于配置 ES8156 寄存器

- **I2S 音频接口**：
  - SSI_TX (音频数据输出)
  - SSI_BCK (位时钟)
  - SSI_LRCK (左右声道时钟)
  - SSI_MCK (主时钟,可选)

## 运行效果

将歌曲转为2声道16k采样率wav然后存入SD卡，再把SD卡插入开发板，在上电则会自动挂载，然后输入命令wavplay -s 文件名即可播放

![alt text](figures/image1.png)

## 相关资料

- [RT-Thread 音频框架文档](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/audio/audio)

