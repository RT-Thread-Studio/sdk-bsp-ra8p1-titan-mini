# IMU 传感器示例说明

**中文** | [**English**](./README.md)

## 简介

本示例展示了如何在 **Titan Board Mini** 上使用 **LSM6DS3TR-C 六轴 IMU 传感器** 实现惯性测量功能,通过 **I2C 接口**读取 **3轴加速度计** 和 **3轴陀螺仪** 数据,结合 **RT-Thread 传感器框架** 实现完整的传感器数据采集和处理。

主要功能包括：

- 使用 LSM6DS3TR-C 实现 6 轴惯性测量
- 读取 3 轴加速度计数据 (X/Y/Z)
- 读取 3 轴陀螺仪数据 (X/Y/Z)
- 支持多种量程和采样率配置
- 集成 RT-Thread 传感器框架

## 硬件介绍

### 1. LSM6DS3TR-C IMU 传感器

**Titan Board Mini** 支持 **LSM6DS3TR-C** 高性能六轴 IMU 传感器：

| 参数 | 说明 |
|------|------|
| **型号** | LSM6DS3TR-C |
| **制造商** | STMicroelectronics (意法半导体) |
| **类型** | 6 轴 IMU (3轴加速度计 + 3轴陀螺仪) |
| **接口** | I2C / SPI |
| **工作电压** | 1.71V - 3.6V |
| **温度范围** | -40°C ~ +85°C |
| **封装** | 2.5mm x 3mm x 0.83mm LGA-14 |

## 软件架构

### 1. 分层设计

IMU 传感器系统采用分层架构：

```
应用程序层 (用户代码)
    ↓
RT-Thread Sensor Driver Framework - 传感器设备框架
    ↓
LSM6DS3TR-C Driver - IMU驱动
```

### 2. 核心组件

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

```
int lsm6ds3tr_c_read_data_sample(void)
{
    /* Initialize mems driver interface */
    stmdev_ctx_t dev_ctx;
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;
    dev_ctx.mdelay = platform_delay;
    /* Init test platform */
    platform_init();
    /* Check device ID */
    whoamI = 0;
    lsm6ds3tr_c_device_id_get(&dev_ctx, &whoamI);

    if (whoamI != LSM6DS3TR_C_ID)
        while (1); /*manage here device not found */

    /* Restore default configuration */
    lsm6ds3tr_c_reset_set(&dev_ctx, PROPERTY_ENABLE);

    do
    {
        lsm6ds3tr_c_reset_get(&dev_ctx, &rst);
    }
    while (rst);

    /* Enable Block Data Update */
    lsm6ds3tr_c_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
    /* Set Output Data Rate */
    lsm6ds3tr_c_xl_data_rate_set(&dev_ctx, LSM6DS3TR_C_XL_ODR_12Hz5);
    lsm6ds3tr_c_gy_data_rate_set(&dev_ctx, LSM6DS3TR_C_GY_ODR_12Hz5);
    /* Set full scale */
    lsm6ds3tr_c_xl_full_scale_set(&dev_ctx, LSM6DS3TR_C_2g);
    lsm6ds3tr_c_gy_full_scale_set(&dev_ctx, LSM6DS3TR_C_2000dps);
    /* Configure filtering chain(No aux interface) */
    /* Accelerometer - analog filter */
    lsm6ds3tr_c_xl_filter_analog_set(&dev_ctx,
                                     LSM6DS3TR_C_XL_ANA_BW_400Hz);
    /* Accelerometer - LPF1 path ( LPF2 not used )*/
    /* Accelerometer - LPF1 + LPF2 path */
    lsm6ds3tr_c_xl_lp2_bandwidth_set(&dev_ctx,
                                     LSM6DS3TR_C_XL_LOW_NOISE_LP_ODR_DIV_100);
    /* Accelerometer - High Pass / Slope path */
    /* Gyroscope - filtering chain */
    lsm6ds3tr_c_gy_band_pass_set(&dev_ctx,
                                 LSM6DS3TR_C_HP_260mHz_LP1_STRONG);

    /* Read samples in polling mode (no int) */
    while (1)
    {
        /* Read output only if new value is available */
        lsm6ds3tr_c_reg_t reg;
        lsm6ds3tr_c_status_reg_get(&dev_ctx, &reg.status_reg);
        if (reg.status_reg.xlda)
        {
            /* Read magnetic field data */
            rt_memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
            lsm6ds3tr_c_acceleration_raw_get(&dev_ctx,
                                             data_raw_acceleration);
            acceleration_mg[0] = lsm6ds3tr_c_from_fs2g_to_mg(
                                     data_raw_acceleration[0]);
            acceleration_mg[1] = lsm6ds3tr_c_from_fs2g_to_mg(
                                     data_raw_acceleration[1]);
            acceleration_mg[2] = lsm6ds3tr_c_from_fs2g_to_mg(
                                     data_raw_acceleration[2]);
            rt_kprintf("Acceleration [mg]:%4.2f\t%4.2f\t%4.2f\r\n",
                       acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);

        }

        if (reg.status_reg.gda)
        {
            /* Read magnetic field data */
            rt_memset(data_raw_angular_rate, 0x00, 3 * sizeof(int16_t));
            lsm6ds3tr_c_angular_rate_raw_get(&dev_ctx,
                                             data_raw_angular_rate);
            angular_rate_mdps[0] = lsm6ds3tr_c_from_fs2000dps_to_mdps(
                                       data_raw_angular_rate[0]);
            angular_rate_mdps[1] = lsm6ds3tr_c_from_fs2000dps_to_mdps(
                                       data_raw_angular_rate[1]);
            angular_rate_mdps[2] = lsm6ds3tr_c_from_fs2000dps_to_mdps(
                                       data_raw_angular_rate[2]);
            rt_kprintf("Angular rate [mdps]:%4.2f\t%4.2f\t%4.2f\r\n",
                       angular_rate_mdps[0], angular_rate_mdps[1], angular_rate_mdps[2]);
        }

        if (reg.status_reg.tda)
        {
            /* Read temperature data */
            rt_memset(&data_raw_temperature, 0x00, sizeof(int16_t));
            lsm6ds3tr_c_temperature_raw_get(&dev_ctx, &data_raw_temperature);
            temperature_degC = lsm6ds3tr_c_from_lsb_to_celsius(
                                   data_raw_temperature);
            rt_kprintf("Temperature [degC]:%6.2f\r\n",
                       temperature_degC);
        }
        rt_thread_mdelay(500);
    }
}
INIT_APP_EXPORT(lsm6ds3tr_c_read_data_sample);
```

程序在上电运行后会自行运行该函数，此时连接上串口终端即可查看打印的IMU数据

## 配置说明

### 1. Kconfig 配置

在 `libraries/M85_Config/Kconfig` 中包含了 IMU 选项：

```kconfig
config BSP_USING_LSM6DS3
    bool "Enable LSM6DS3 6-axis IMU"
    default n
```

### 2. RT-Thread Settings

在 RT-Thread Studio 中,需要启用以下组件：

1. **设备驱动**
   - 启用 I2C 设备驱动
   - 配置 I2C1 接口

2. **传感器**
   - 启用 LSM6DS3 6-axis IMU

## 运行效果

### 1. 终端输出

复位 Titan Board Mini 后终端会输出如下信息：

![image1](figures/image1.png)

## 相关资料

- [RT-Thread 传感器框架文档](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/sensor/sensor)
