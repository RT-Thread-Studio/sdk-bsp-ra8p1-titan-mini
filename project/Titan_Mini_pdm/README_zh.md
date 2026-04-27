# PDM 麦克风实时回环示例说明

**中文** | [**English**](./README.md)

## 简介

本示例展示了如何在 **Titan Board Mini** 上使用 **RA8P1 的 PDM (Pulse Density Modulation) 接口** 读取 **PDM 麦克风** 数据，并通过 **ES8156 音频编解码器** 实时播放，实现 **麦克风 → 扬声器** 的音频回环功能。

主要功能包括：

- 使用 RA8P1 内置 PDM 接口读取 PDM 麦克风数据
- PDM 数据转 PCM 格式（支持 16-bit 和 20-bit 模式）
- 单声道转双声道音频处理
- 双缓冲机制实现连续音频流播放
- 通过 SSI 接口和 ES8156 编解码器输出音频

## 硬件介绍

### 1. PDM 麦克风

**Titan Board Mini** 板载 **PDM 麦克风**：

| 参数 | 说明 |
|------|------|
| **类型** | MEMS PDM 麦克风 |
| **接口** | PDM (Pulse Density Modulation) |
| **输出格式** | 1-bit 密度流 |
| **采样率** | 最高 16kHz（本示例） |
| **声道** | 单声道 |

### 2. RA8P1 PDM 接口

RA8P1 的 PDM 接口功能：

- **内置 PDM 解调器**：硬件实现 PDM → PCM 转换
- **可编程滤波器**：SINC 滤波器、HPF、LPF
- **多通道支持**：支持多路麦克风输入
- **可配置输出**：支持 16-bit 或 20-bit PCM 输出
- **DMA 支持**：支持 DMA 传输，减少 CPU 占用

### 3. 音频输出

- **编解码器**：ES8156（24-bit DAC，支持 8kHz - 192kHz）
- **接口**：SSI (I2S 协议)
- **输出**：3.5mm 耳机接口 / 板载扬声器

## 软件架构

### 1. 数据流程

```
PDM 麦克风
    ↓ (PDM 信号)
RA8P1 PDM 接口
    ↓ (硬件滤波 + PDM→PCM)
PDM 缓冲区 (int32_t[])
    ↓ (软件转换)
DAC 缓冲区 (int16_t[], 双声道)
    ↓ (I2S 协议)
ES8156 编解码器
    ↓ (模拟音频)
扬声器/耳机
```

### 2. 核心组件

#### PDM 驱动

使用 Renesas FSP 的 PDM 驱动：

```c
/* 打开 PDM 模块 */
R_PDM_Open(&g_pdm0_ctrl, &g_pdm0_cfg);

/* 启动数据采集 */
R_PDM_Start(&g_pdm0_ctrl, buffer, buf_size.bytes, samples);

/* 停止采集 */
R_PDM_Stop(&g_pdm0_ctrl);

/* 关闭 PDM 模块 */
R_PDM_Close(&g_pdm0_ctrl);
```

#### 数据转换函数

```c
/* PDM 数据转 DAC 数据 */
static void convert_pdm_to_dac(int32_t *pdm_data, uint32_t pdm_samples, int16_t *dac_data)
{
    for (uint32_t i = 0; i < pdm_samples; i++)
    {
        int16_t sample_16;
        if (g_pcm_bits == PCM_20BITS)
        {
            /* 20 位 -> 16 位：右移 4 位 */
            sample_16 = (int16_t)(pdm_data[i] >> 4);
        }
        else
        {
            /* 16 位模式：左移 1 位 */
            sample_16 = (int16_t)(pdm_data[i] << 1);
        }

        /* 单声道转双声道 */
        dac_data[i * 2] = sample_16;      /* 左声道 */
        dac_data[i * 2 + 1] = sample_16;  /* 右声道 */
    }
}
```

### 3. 双缓冲机制

为了实现连续音频流，采用双缓冲模式：

```
时间轴：
    t0      t1      t2      t3      t4
    |-------|-------|-------|-------|
缓冲区0: [录音 ] [播放         ]
缓冲区1:                 [录音 ] [播放         ]
```

- **缓冲区 0**：录音 → 播放
- **缓冲区 1**：录音 → 播放
- 两个缓冲区交替工作，实现无缝连续播放

### 4. 工程结构

```
Titan_Mini_pdm/
├── src/
│   └── hal_entry.c          # 主程序入口
├── libraries/
│   └── HAL_Drivers/
│       ├── drv_i2s.c        # SSI/I2S 驱动
│       └── ports/
│           └── es8156/      # ES8156 编解码器驱动
└── ra_gen/
    └── hal_data.c           # FSP 配置（PDM、SSI 等）
```

## 配置说明

### 1. PDM 配置

在 `ra_gen/hal_data.c` 中配置 PDM 参数：

```c
const pdm_cfg_t g_pdm0_cfg =
{
    .unit                              = 0,
    .channel                           = 2,        /* PDM 通道 */
    .pcm_width                         = PDM_PCM_WIDTH_16_BITS_0_14,  /* 16-bit 输出 */
    .pcm_edge                          = PDM_INPUT_DATA_EDGE_RISE,
    .p_extend                          = &g_pdm0_cfg_extend,
    // ...
};
```

### 2. 关键参数

| 参数 | 值 | 说明 |
|------|-----|------|
| `PDM_FS_HZ` | 16000 | 输出采样率 16kHz |
| `PDM_CHANNELS` | 1 | 单声道输入 |
| `PDM_PCM_WIDTH` | 16_BITS_0_14 | 16-bit PCM，有效位 [14:0] |
| `NUM_BUFFERS` | 2 | 双缓冲模式 |
| `PDM_RECORD_DURATION_SEC` | 1 | 每个缓冲区 1 秒音频 |

## 运行效果

![运行效果](figures/image1.png)

1. 上电后，绿色 LED 点亮表示系统就绪
2. 蓝色 LED 点亮表示音频回环正在运行
3. 对着麦克风说话，可以在耳机/扬声器中实时听到声音
4. 蓝色 LED 每 50 次循环闪烁一次（心跳指示）

## 故障排查

### 1. 没有声音输出

- 检查耳机/扬声器是否正确连接
- 检查音量设置（默认 60%）
- 检查 ES8156 初始化是否成功

### 2. 声音卡顿

- 当前使用 1 秒缓冲区，可尝试调整 `PDM_RECORD_DURATION_SEC`
- 检查系统负载，减少其他任务

### 3. 声音失真

- 检查 `pcm_width` 配置是否与硬件匹配
- 调整数据转换函数中的位移参数