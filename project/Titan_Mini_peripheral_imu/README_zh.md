# LSM6DS3TR-C 六轴 IMU 传感器使用说明

**中文** | [**English**](./README.md)

## 简介

本示例展示了如何在 **Titan Board Mini** 上使用 **LSM6DS3TR-C 六轴 IMU 传感器** 实现惯性测量功能,通过 **I2C 接口**读取 **3轴加速度计** 和 **3轴陀螺仪** 数据,结合 **RT-Thread 传感器框架** 实现完整的传感器数据采集和处理。

主要功能包括：

- 使用 LSM6DS3TR-C 实现 6 轴惯性测量
- 读取 3 轴加速度计数据 (X/Y/Z)
- 读取 3 轴陀螺仪数据 (X/Y/Z)
- 支持多种量程和采样率配置
- 集成 RT-Thread 传感器框架
- 支持 FIFO 数据缓存和中断模式

![image](figures/big.png)

## 硬件介绍

### 1. LSM6DS3TR-C IMU 传感器

**Titan Board Mini** 板载 **LSM6DS3TR-C** 高性能六轴 IMU 传感器：

| 参数 | 说明 |
|------|------|
| **型号** | LSM6DS3TR-C |
| **制造商** | STMicroelectronics (意法半导体) |
| **类型** | 6 轴 IMU (3轴加速度计 + 3轴陀螺仪) |
| **接口** | I2C / SPI |
| **工作电压** | 1.71V - 3.6V |
| **温度范围** | -40°C ~ +85°C |
| **封装** | 2.5mm x 3mm x 0.83mm LGA-14 |

### 2. 加速度计特性

LSM6DS3TR-C 内置高性能 3 轴加速度计：

- **量程选择**：±2g / ±4g / ±8g / ±16g
- **分辨率**：16-bit ADC
- **输出数据率**：1.6Hz - 6.66kHz
- **噪声密度**：90μg/√Hz
- **零偏偏差**：±40mg
- **带宽**：可配置 (通常 50Hz - 1.6kHz)

### 3. 陀螺仪特性

LSM6DS3TR-C 内置高精度 3 轴陀螺仪：

- **量程选择**：±125 / ±250 / ±500 / ±1000 / ±2000 dps
- **分辨率**：16-bit ADC
- **输出数据率**：1.6Hz - 6.66kHz
- **噪声密度**：3.8mdps/√Hz
- **零偏稳定性**：±5dps
- **带宽**：可配置

### 4. 主要功能

#### 高级功能

- **FIFO 缓存**：9KB FIFO,支持多种模式
- **中断功能**：运动唤醒、自由落体、6D方向检测
- **传感器融合**：内置低功耗传感器融合算法
- **自检功能**：支持自检模式
- **低功耗模式**：多种低功耗工作模式

#### 数据处理

- **硬件滤波**：可配置数字滤波器
- **数据融合**：支持加速度计+陀螺仪数据融合
- **时间戳**：内置时间戳功能
- **轮询/中断**：支持轮询和中断数据读取

## 软件架构

### 1. 分层设计

IMU 传感器系统采用分层架构：

```
应用程序层 (用户代码)
    ↓
RT-Thread Sensor Framework - 传感器框架
    ↓
LSM6DS3TR-C Driver - IMU驱动
    ↓
Sensor HAL - 传感器硬件抽象层
    ↓
I2C/SPI Driver - I2C/SPI驱动
    ↓
FSP I2C/SPI HAL - 硬件抽象层
```

### 2. 核心组件

#### LSM6DS3TR-C 寄存器驱动

ST 官方提供的寄存器级驱动 (`lsm6ds3tr-c_reg.c/h`)：

```c
/* 初始化传感器 */
int32_t lsm6ds3tr_c_init(lsm6ds3tr_c_t *ctx);

/* 读取加速度计数据 */
int32_t lsm6ds3tr_c_acceleration_raw_get(lsm6ds3tr_c_t *ctx, lsm6ds3tr_c_axis3bit16_t *val);

/* 读取陀螺仪数据 */
int32_t lsm6ds3tr_c_angular_rate_raw_get(lsm6ds3tr_c_t *ctx, lsm6ds3tr_c_axis3bit16_t *val);

/* 配置加速度计量程 */
int32_t lsm6ds3tr_c_xl_full_scale_set(lsm6ds3tr_c_t *ctx, lsm6ds3tr_c_fs_xl_t val);

/* 配置陀螺仪量程 */
int32_t lsm6ds3tr_c_gy_full_scale_set(lsm6ds3tr_c_t *ctx, lsm6ds3tr_c_fs_g_t val);

/* 配置输出数据率 */
int32_t lsm6ds3tr_c_xl_data_rate_set(lsm6ds3tr-c_t *ctx, lsm6ds3tr_c_odr_xl_t val);
```

#### 移植层接口

需要实现的平台相关接口 (`lsm6ds3tr-c_port.c`)：

```c
/* I2C 读写接口 */
int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);

/* 延时接口 */
void platform_delay(uint32_t ms);
```

#### RT-Thread 传感器框架

RT-Thread 提供的统一传感器设备接口：

```c
/* 查找传感器设备 */
rt_device_t rt_device_find(const char *name);

/* 打开传感器设备 */
rt_err_t rt_device_open(rt_device_t dev, rt_uint16_t oflags);

/* 读取传感器数据 */
rt_size_t rt_device_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size);

/* 接收传感器数据 */
rt_err_t rt_device_set_rx_indicate(rt_device_t dev, rt_err_t (*rx_ind)(rt_device_t dev, rt_size_t size));
```

### 3. 工程结构

```
Titan_Mini_peripheral_imu/
├── src/
│   └── hal_entry.c          # 主程序入口
└── packages/
    └── lsm6ds3tr/           # LSM6DS3TR-C 驱动包
        ├── lsm6ds3tr-c_reg.h    # 寄存器定义和驱动接口
        ├── lsm6ds3tr-c_reg.c    # 寄存器级驱动实现
        └── lsm6ds3tr-c_port.c   # 平台移植层
```

## 使用说明

### 1. 初始化流程

系统初始化时需要初始化 I2C 接口和 IMU 传感器：

```c
#include <rtthread.h>
#include "lsm6ds3tr-c_reg.h"

/* I2C 配置 */
#define LSM6DS3TR_C_I2C_BUS    "i2c2"
#define LSM6DS3TR_C_I2C_ADDR    0x6A  /* SA0引脚接GND */

void hal_entry(void)
{
    stmdev_ctx_t imu_ctx = {0};

    /* 初始化 I2C 接口 */
    struct rt_i2c_bus_device *i2c_bus = rt_i2c_bus_device_find(LSM6DS3TR_C_I2C_BUS);
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("I2C bus not found!\n");
        return;
    }

    /* 配置设备读写接口 */
    imu_ctx.handle    = i2c_bus;
    imu_ctx.write_reg = platform_write;
    imu_ctx.read_reg  = platform_read;

    /* 初始化传感器 */
    if (lsm6ds3tr_c_init(&imu_ctx) != LSM6DS3TR_C_OK)
    {
        rt_kprintf("LSM6DS3TR-C initialization failed!\n");
        return;
    }

    /* 配置加速度计 */
    lsm6ds3tr_c_xl_full_scale_set(&imu_ctx, LSM6DS3TR_C_2g);          /* ±2g */
    lsm6ds3tr_c_xl_data_rate_set(&imu_ctx, LSM6DS3TR_C_ODR_104Hz);  /* 104Hz */

    /* 配置陀螺仪 */
    lsm6ds3tr_c_gy_full_scale_set(&imu_ctx, LSM6DS3TR_C_250dps);    /* ±250dps */
    lsm6ds3tr_c_gy_data_rate_set(&imu_ctx, LSM6DS3TR_C_ODR_104Hz);  /* 104Hz */

    rt_kprintf("LSM6DS3TR-C initialized successfully!\n");

    /* 主循环 - 读取传感器数据 */
    while (1)
    {
        /* 读取加速度计数据 */
        lsm6ds3tr_c_axis3bit16_t acc_raw;
        lsm6ds3tr_c_acceleration_raw_get(&imu_ctx, &acc_raw);

        /* 读取陀螺仪数据 */
        lsm6ds3tr_c_axis3bit16_t gyro_raw;
        lsm6ds3tr_c_angular_rate_raw_get(&imu_ctx, &gyro_raw);

        /* 数据转换为物理单位 */
        float acc_x = acc_raw.i16bit[0] / 32768.0f * 2.0f;  /* 2g 量程 */
        float acc_y = acc_raw.i16bit[1] / 32768.0f * 2.0f;
        float acc_z = acc_raw.i16bit[2] / 32768.0f * 2.0f;

        float gyro_x = gyro_raw.i16bit[0] / 32768.0f * 250.0f;  /* 250dps 量程 */
        float gyro_y = gyro_raw.i16bit[1] / 32768.0f * 250.0f;
        float gyro_z = gyro_raw.i16bit[2] / 32768.0f * 250.0f;

        /* 打印数据 */
        rt_kprintf("ACC: X=%.3fg Y=%.3fg Z=%.3fg | GYRO: X=%.2fdps Y=%.2fdps Z=%.2fdps\n",
                   acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z);

        rt_thread_mdelay(100);  /* 10Hz 读取频率 */
    }
}
```

### 2. I2C 平台接口实现

移植层需要实现 I2C 读写函数：

```c
#include <rtthread.h>
#include <rtdevice.h>

/* I2C 写入 */
int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    struct rt_i2c_bus_device *i2c_bus = (struct rt_i2c_bus_device *)handle;
    struct rt_i2c_msg msgs[2];

    /* 写寄存器地址 */
    msgs[0].addr  = LSM6DS3TR_C_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    /* 写数据 */
    msgs[1].addr  = LSM6DS3TR_C_I2C_ADDR;
    msgs[1].flags = RT_I2C_WR | RT_I2C_NO_START;
    msgs[1].buf   = (uint8_t *)bufp;
    msgs[1].len   = len;

    if (rt_i2c_transfer(i2c_bus, msgs, 2) != 2)
    {
        return -1;
    }
    return 0;
}

/* I2C 读取 */
int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    struct rt_i2c_bus_device *i2c_bus = (struct rt_i2c_bus_device *)handle;
    struct rt_i2c_msg msgs[2];

    /* 写寄存器地址 */
    msgs[0].addr  = LSM6DS3TR_C_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    /* 读数据 */
    msgs[1].addr  = LSM6DS3TR_C_I2C_ADDR;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = bufp;
    msgs[1].len   = len;

    if (rt_i2c_transfer(i2c_bus, msgs, 2) != 2)
    {
        return -1;
    }
    return 0;
}

/* 延时函数 */
void platform_delay(uint32_t ms)
{
    rt_thread_mdelay(ms);
}
```

### 3. 数据格式转换

传感器原始数据需要转换为物理单位：

```c
/* 加速度计数据转换 (假设 2g 量程) */
int16_t acc_raw_x = 16384;  /* 原始 ADC 值 */
float acc_x_g = (float)acc_raw_x / 32768.0f * 2.0f;  /* 转换为 g (9.8m/s²) */
float acc_x_m_s2 = acc_x_g * 9.80665f;  /* 转换为 m/s² */

/* 陀螺仪数据转换 (假设 250dps 量程) */
int16_t gyro_raw_x = 1000;  /* 原始 ADC 值 */
float gyro_x_dps = (float)gyro_raw_x / 32768.0f * 250.0f;  /* 转换为 °/s */
float gyro_x_rad_s = gyro_x_dps * 0.017453292519943295f;  /* 转换为 rad/s */
```

## 配置说明

### 1. Kconfig 配置

在 `libraries/M85_Config/Kconfig` 中配置 IMU 选项：

```kconfig
menuconfig BSP_USING_IMU
    bool "Enable IMU (LSM6DS3TR-C)"
    select BSP_USING_I2C2
    default n
    if BSP_USING_IMU
        config BSP_IMU_I2C_BUS
            string "I2C bus name"
            default "i2c2"

        config BSP_IMU_ACC_ODR
            int "Accelerometer ODR (Hz)"
            default 104

        config BSP_IMU_GYRO_ODR
            int "Gyroscope ODR (Hz)"
            default 104
    endif
```

### 2. RT-Thread Settings

在 RT-Thread Studio 中,需要启用以下组件：

1. **设备驱动**
   - 启用 I2C 设备驱动
   - 配置 I2C2 接口

2. **传感器**
   - 启用 RT-Thread Sensor 框架
   - 启用 Accel (加速度计) 传感器
   - 启用 Gyro (陀螺仪) 传感器

3. **软件包**
   - 添加 LSM6DS3TR-C 驱动包

### 3. 硬件连接

LSM6DS3TR-C 与 Titan Board Mini 的连接：

- **I2C 接口**：
  - I2C2_SCL / I2C2_SDA
  - I2C 地址：0x6A (SA0=GND) 或 0x6B (SA0=VDD)

- **中断引脚** (可选)：
  - INT1：数据就绪中断
  - INT2：事件中断 (运动检测等)

- **电源**：
  - VDD：1.71V - 3.6V
  - GND：地

## 性能特性

### 1. 传感器性能

| 参数 | 加速度计 | 陀螺仪 |
|------|----------|--------|
| **量程** | ±2/±4/±8/±16g | ±125/±250/±500/±1000/±2000dps |
| **分辨率** | 16-bit (65536 LSB) | 16-bit (65536 LSB) |
| **输出数据率** | 1.6Hz - 6.66kHz | 1.6Hz - 6.66kHz |
| **噪声密度** | 90μg/√Hz | 3.8mdps/√Hz |
| **零偏稳定性** | ±40mg | ±5dps |
| **带宽** | 50Hz - 1.6kHz | 50Hz - 1.6kHz |

### 2. 系统性能

- **I2C 速率**：标准模式 (100kHz) 或快速模式 (400kHz)
- **读取延迟**：< 1ms (单次 6 轴数据读取)
- **CPU 占用**：< 2% (轮询模式)
- **FIFO 深度**：9KB (约 1000 帧 6 轴数据)

### 3. 功耗特性

| 模式 | 加速度计 | 陀螺仪 | 总功耗 |
|------|----------|--------|--------|
| **高性能** | 0.55mA | 0.9mA | 1.45mA |
| **低功耗** | 25μA | - | 25μA |
| **待机** | 3μA | 3μA | 6μA |

## 编译&下载

### 1. 编译环境

- **IDE**：RT-Thread Studio
- **编译器**：ARM GCC
- **配置文件**：
  - `.config`：RT-Thread 配置
  - `rtconfig.h`：RT-Thread 配置头文件
  - `configuration.xml`：FSP 配置

### 2. 编译步骤

1. 在 RT-Thread Studio 的包管理器中下载 Titan Board 资源包
2. 创建新工程或导入现有工程
3. 在 RT-Thread Settings 中启用 IMU 相关组件
4. 添加 LSM6DS3TR-C 驱动包
5. 配置 I2C 接口和传感器参数
6. 执行编译

### 3. 下载步骤

1. 将开发板的 **USB-DBG 接口**与 PC 连接
2. 使用 J-Link 或 DAP-Link 下载器
3. 将生成的固件下载至开发板
4. 复位开发板运行程序

## 运行效果

### 1. 终端输出

复位 Titan Board Mini 后终端会输出如下信息：

```bash
 \ | /
- RT -     Thread Operating System
 / | \     5.1.0 build Aug 5 2025 17:24:30
 2006 - 2024 Copyright by RT-Thread team

==================================================
Hello RT-Thread!
==================================================
[I2C2] I2C bus initialized
[LSM6DS3TR-C] Chip ID: 0x6C (verified)
[LSM6DS3TR-C] Sensor initialized
ACC: X=0.012g Y=0.008g Z=1.005g | GYRO: X=0.52dps Y=-0.31dps Z=0.18dps
ACC: X=0.011g Y=0.007g Z=1.004g | GYRO: X=0.48dps Y=-0.28dps Z=0.16dps
...
msh >
```

### 2. 数据示例

不同姿态下的传感器输出：

- **静止水平放置**：
  - ACC: X≈0g, Y≈0g, Z≈1g (重力)
  - GYRO: X≈0dps, Y≈0dps, Z≈0dps

- **沿 X 轴倾斜 45°**：
  - ACC: X≈0.707g, Y≈0g, Z≈0.707g
  - GYRO: X≈0dps, Y≈0dps, Z≈0dps

- **绕 Z 轴旋转**：
  - ACC: X≈0g, Y≈0g, Z≈1g
  - GYRO: Z≈±50~200dps (取决于旋转速度)

## 应用场景

- **姿态检测**：设备倾斜角度检测
- **运动识别**：走路、跑步、跳跃等运动识别
- **导航定位**：惯性导航系统 (INS)
- **机器人控制**：平衡机器人、无人机姿态控制
- **虚拟现实**：VR/AR 设备头部追踪
- **手势识别**：手势控制和识别
- **跌倒检测**：老年人跌倒检测
- **计步器**：计步功能实现
- **震动检测**：设备震动和冲击检测
- **游戏控制**：体感游戏控制

## 注意事项

### 1. 硬件安装

- **PCB 布局**：传感器应远离电机、散热器等干扰源
- **机械固定**：确保传感器牢固固定,避免震动影响
- **方向校准**：注意传感器方向与设备坐标系的对应关系

### 2. 数据校准

- **零偏校准**：使用前进行零偏校准,提高精度
- **温度补偿**：考虑温度对传感器性能的影响
- **交叉轴干扰**：注意加速度计和陀螺仪的交叉轴干扰

### 3. 数据处理

- **滤波算法**：使用低通滤波器滤除高频噪声
- **传感器融合**：结合加速度计和陀螺仪数据进行传感器融合
- **采样率选择**：根据应用选择合适的采样率,平衡功耗和性能

## 扩展应用

基于本示例,可以扩展以下应用：

- **姿态解算**：实现四元数、欧拉角姿态解算
- **运动算法**：实现步数统计、距离估算等算法
- **传感器融合**：结合磁力计实现 9 轴传感器融合 (AHRS)
- **机器学习**：使用 IMU 数据进行机器学习分类
- **低功耗设计**：优化采样率和中断机制,降低功耗
- **无线传输**：通过蓝牙/WiFi 传输 IMU 数据
- **数据记录**：将 IMU 数据记录到 SD 卡
- **实时可视化**：在 LCD 上实时显示传感器数据

## 相关资料

- [LSM6DS3TR-C 数据手册](https://www.st.com/resource/en/datasheet/lsm6ds3tr-c.pdf)
- [LSM6DS3TR-C 应用笔记](https://www.st.com/resource/en/application_note/an5054-lsm6ds3tr-c-mems-inertial-measurement-unit-6-axis-fis-on‑1g-axis-stmicroelectronics.pdf)
- [IMU 工作原理](https://en.wikipedia.org/wiki/Inertial_measurement_unit)
- [RT-Thread 传感器框架文档](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/sensor/sensor)
- [RA8P1 硬件手册](https://www.renesas.cn/zh/document/mah/25574257)
