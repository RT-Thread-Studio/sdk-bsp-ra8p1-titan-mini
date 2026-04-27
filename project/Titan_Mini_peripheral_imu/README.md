# IMU Sensor Example Guide

[**中文**](./README_zh.md) | **English**

## Introduction

This example demonstrates how to use the **LSM6DS3TR-C 6-axis IMU sensor** on **Titan Board Mini** to implement inertial measurement functionality. It reads **3-axis accelerometer** and **3-axis gyroscope** data via the **I2C interface**, combined with the **RT-Thread Sensor Framework** for complete sensor data acquisition and processing.

Key features include:

- 6-axis inertial measurement using LSM6DS3TR-C
- Read 3-axis accelerometer data (X/Y/Z)
- Read 3-axis gyroscope data (X/Y/Z)
- Support for multiple range and sampling rate configurations
- Integrated with RT-Thread Sensor Framework

## Hardware Overview

### 1. LSM6DS3TR-C IMU Sensor

**Titan Board Mini** features the onboard **LSM6DS3TR-C** high-performance 6-axis IMU sensor:

| Parameter | Description |
|-----------|-------------|
| **Model** | LSM6DS3TR-C |
| **Manufacturer** | STMicroelectronics |
| **Type** | 6-axis IMU (3-axis accelerometer + 3-axis gyroscope) |
| **Interface** | I2C / SPI |
| **Operating Voltage** | 1.71V - 3.6V |
| **Temperature Range** | -40°C ~ +85°C |
| **Package** | 2.5mm x 3mm x 0.83mm LGA-14 |

### 2. Accelerometer Features

The LSM6DS3TR-C integrates a high-performance 3-axis accelerometer:

- **Range Selection**: ±2g / ±4g / ±8g / ±16g
- **Resolution**: 16-bit ADC
- **Output Data Rate**: 1.6Hz - 6.66kHz
- **Noise Density**: 90μg/√Hz
- **Zero-g Offset**: ±40mg
- **Bandwidth**: Configurable (typically 50Hz - 1.6kHz)

### 3. Gyroscope Features

The LSM6DS3TR-C integrates a high-precision 3-axis gyroscope:

- **Range Selection**: ±125 / ±250 / ±500 / ±1000 / ±2000 dps
- **Resolution**: 16-bit ADC
- **Output Data Rate**: 1.6Hz - 6.66kHz
- **Noise Density**: 3.8mdps/√Hz
- **Bias Stability**: ±5dps
- **Bandwidth**: Configurable

### 4. Main Features

#### Advanced Features

- **FIFO Buffer**: 9KB FIFO with multiple modes
- **Interrupt Functions**: Motion wake-up, free fall, 6D orientation detection
- **Sensor Fusion**: Built-in low-power sensor fusion algorithms
- **Self-Test**: Supports self-test mode
- **Low Power Mode**: Multiple low-power operating modes

#### Data Processing

- **Hardware Filtering**: Configurable digital filters
- **Data Fusion**: Supports accelerometer + gyroscope data fusion
- **Time Stamp**: Built-in timestamp function
- **Polling/Interrupt**: Supports polling and interrupt data reading

## Software Architecture

### 1. Layered Design

The IMU sensor system uses a layered architecture:

```
Application Layer (User Code)
    ↓
RT-Thread Sensor Framework - Sensor Framework
    ↓
LSM6DS3TR-C Driver - IMU Driver
    ↓
Sensor HAL - Sensor Hardware Abstraction Layer
    ↓
I2C/SPI Driver - I2C/SPI Driver
    ↓
FSP I2C/SPI HAL - Hardware Abstraction Layer
```

### 2. Core Components

#### Porting Layer Interface

Platform-specific interfaces to implement (`lsm6ds3tr-c_port.c`):

```c
/* I2C read/write interfaces */
int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);

/* Delay interface */
void platform_delay(uint32_t ms);
```

#### RT-Thread Sensor Framework

Unified sensor device interface provided by RT-Thread:

```c
/* Find sensor device */
rt_device_t rt_device_find(const char *name);

/* Open sensor device */
rt_err_t rt_device_open(rt_device_t dev, rt_uint16_t oflags);

/* Read sensor data */
rt_size_t rt_device_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size);

/* Receive sensor data */
rt_err_t rt_device_set_rx_indicate(rt_device_t dev, rt_err_t (*rx_ind)(rt_device_t dev, rt_size_t size));
```

### 3. Project Structure

```
Titan_Mini_peripheral_imu/
├── src/
│   └── hal_entry.c          # Main program entry
└── packages/
    └── lsm6ds3tr/           # LSM6DS3TR-C driver package
        ├── lsm6ds3tr-c_reg.h    # Register definitions and driver interfaces
        ├── lsm6ds3tr-c_reg.c    # Register-level driver implementation
        └── lsm6ds3tr-c_port.c   # Platform porting layer
```

## Usage Guide

### 1. Initialization Flow

The system requires initialization of the I2C interface and IMU sensor during startup:

```c
#include <rtthread.h>
#include "lsm6ds3tr-c_reg.h"

/* I2C configuration */
#define LSM6DS3TR_C_I2C_BUS    "i2c2"
#define LSM6DS3TR_C_I2C_ADDR    0x6A  /* SA0 pin connected to GND */

void hal_entry(void)
{
    stmdev_ctx_t imu_ctx = {0};

    /* Initialize I2C interface */
    struct rt_i2c_bus_device *i2c_bus = rt_i2c_bus_device_find(LSM6DS3TR_C_I2C_BUS);
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("I2C bus not found!\n");
        return;
    }

    /* Configure device read/write interfaces */
    imu_ctx.handle    = i2c_bus;
    imu_ctx.write_reg = platform_write;
    imu_ctx.read_reg  = platform_read;

    /* Initialize sensor */
    if (lsm6ds3tr_c_init(&imu_ctx) != LSM6DS3TR_C_OK)
    {
        rt_kprintf("LSM6DS3TR-C initialization failed!\n");
        return;
    }

    /* Configure accelerometer */
    lsm6ds3tr_c_xl_full_scale_set(&imu_ctx, LSM6DS3TR_C_2g);          /* ±2g */
    lsm6ds3tr_c_xl_data_rate_set(&imu_ctx, LSM6DS3TR_C_ODR_104Hz);  /* 104Hz */

    /* Configure gyroscope */
    lsm6ds3tr_c_gy_full_scale_set(&imu_ctx, LSM6DS3TR_C_250dps);    /* ±250dps */
    lsm6ds3tr_c_gy_data_rate_set(&imu_ctx, LSM6DS3TR_C_ODR_104Hz);  /* 104Hz */

    rt_kprintf("LSM6DS3TR-C initialized successfully!\n");

    /* Main loop - read sensor data */
    while (1)
    {
        /* Read accelerometer data */
        lsm6ds3tr_c_axis3bit16_t acc_raw;
        lsm6ds3tr_c_acceleration_raw_get(&imu_ctx, &acc_raw);

        /* Read gyroscope data */
        lsm6ds3tr_c_axis3bit16_t gyro_raw;
        lsm6ds3tr_c_angular_rate_raw_get(&imu_ctx, &gyro_raw);

        /* Convert data to physical units */
        float acc_x = acc_raw.i16bit[0] / 32768.0f * 2.0f;  /* 2g range */
        float acc_y = acc_raw.i16bit[1] / 32768.0f * 2.0f;
        float acc_z = acc_raw.i16bit[2] / 32768.0f * 2.0f;

        float gyro_x = gyro_raw.i16bit[0] / 32768.0f * 250.0f;  /* 250dps range */
        float gyro_y = gyro_raw.i16bit[1] / 32768.0f * 250.0f;
        float gyro_z = gyro_raw.i16bit[2] / 32768.0f * 250.0f;

        /* Print data */
        rt_kprintf("ACC: X=%.3fg Y=%.3fg Z=%.3fg | GYRO: X=%.2fdps Y=%.2fdps Z=%.2fdps\n",
                   acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z);

        rt_thread_mdelay(100);  /* 10Hz read rate */
    }
}
```

### 2. I2C Platform Interface Implementation

The porting layer needs to implement I2C read/write functions:

```c
#include <rtthread.h>
#include <rtdevice.h>

/* I2C write */
int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    struct rt_i2c_bus_device *i2c_bus = (struct rt_i2c_bus_device *)handle;
    struct rt_i2c_msg msgs[2];

    /* Write register address */
    msgs[0].addr  = LSM6DS3TR_C_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    /* Write data */
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

/* I2C read */
int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    struct rt_i2c_bus_device *i2c_bus = (struct rt_i2c_bus_device *)handle;
    struct rt_i2c_msg msgs[2];

    /* Write register address */
    msgs[0].addr  = LSM6DS3TR_C_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    /* Read data */
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

/* Delay function */
void platform_delay(uint32_t ms)
{
    rt_thread_mdelay(ms);
}
```

### 3. Data Format Conversion

Raw sensor data needs to be converted to physical units:

```c
/* Accelerometer data conversion (assuming 2g range) */
int16_t acc_raw_x = 16384;  /* Raw ADC value */
float acc_x_g = (float)acc_raw_x / 32768.0f * 2.0f;  /* Convert to g (9.8m/s²) */
float acc_x_m_s2 = acc_x_g * 9.80665f;  /* Convert to m/s² */

/* Gyroscope data conversion (assuming 250dps range) */
int16_t gyro_raw_x = 1000;  /* Raw ADC value */
float gyro_x_dps = (float)gyro_raw_x / 32768.0f * 250.0f;  /* Convert to °/s */
float gyro_x_rad_s = gyro_x_dps * 0.017453292519943295f;  /* Convert to rad/s */
```

## Configuration Guide

### 1. Kconfig Configuration

Configure IMU options in `libraries/M85_Config/Kconfig`:

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

In RT-Thread Studio, the following components need to be enabled:

1. **Device Drivers**
   - Enable I2C device driver
   - Configure I2C2 interface

2. **Sensors**
   - Enable RT-Thread Sensor Framework
   - Enable Accel (accelerometer) sensor
   - Enable Gyro (gyroscope) sensor

3. **Software Packages**
   - Add LSM6DS3TR-C driver package

## Running Results

### 1. Terminal Output

After resetting Titan Board Mini, the terminal will output the following information:

![image1](figures/image1.png)

## Related Resources

- [RT-Thread Sensor Framework Documentation](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/sensor/sensor)
