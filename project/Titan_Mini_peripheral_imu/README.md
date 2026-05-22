# IMU Sensor Example

[**中文**](./README_zh.md) | **English**

## Overview

This example demonstrates how to use the **LSM6DS3TR-C 6-axis IMU sensor** on the **Titan Board Mini** to implement inertial measurement functionality. It reads **3-axis accelerometer** and **3-axis gyroscope** data through the **I2C interface**, combined with the **RT-Thread sensor framework** to achieve complete sensor data acquisition and processing.

Main features include:

- 6-axis inertial measurement using LSM6DS3TR-C
- Read 3-axis accelerometer data (X/Y/Z)
- Read 3-axis gyroscope data (X/Y/Z)
- Support for multiple range and sampling rate configurations
- Integration with RT-Thread sensor framework

## Hardware Introduction

### 1. LSM6DS3TR-C IMU Sensor

**Titan Board Mini** supports the **LSM6DS3TR-C** high-performance 6-axis IMU sensor:

| Parameter | Description |
|-----------|-------------|
| **Model** | LSM6DS3TR-C |
| **Manufacturer** | STMicroelectronics |
| **Type** | 6-axis IMU (3-axis accelerometer + 3-axis gyroscope) |
| **Interface** | I2C / SPI |
| **Operating Voltage** | 1.71V - 3.6V |
| **Temperature Range** | -40°C ~ +85°C |
| **Package** | 2.5mm x 3mm x 0.83mm LGA-14 |

## Software Architecture

### 1. Layered Design

The IMU sensor system adopts a layered architecture:

```
Application Layer (user code)
    ↓
RT-Thread Sensor Driver Framework - Sensor device framework
    ↓
LSM6DS3TR-C Driver - IMU driver
```

### 2. Core Components

#### Platform Porting Layer Interface

Platform-related interfaces to be implemented (`lsm6ds3tr-c_port.c`):

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
        ├── lsm6ds3tr-c_reg.h    # Register definitions and driver interface
        ├── lsm6ds3tr-c_reg.c    # Register-level driver implementation
        └── lsm6ds3tr-c_port.c   # Platform porting layer
```

## Usage Instructions

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

After the program is powered on, it will automatically run this function. Simply connect a serial terminal to view the printed IMU data.

## Configuration Instructions

### 1. Kconfig Configuration

In `libraries/M85_Config/Kconfig`, IMU options are included:

```kconfig
config BSP_USING_LSM6DS3
    bool "Enable LSM6DS3 6-axis IMU"
    default n
```

### 2. RT-Thread Settings

In RT-Thread Studio, the following components need to be enabled:

1. **Device Drivers**
   - Enable I2C device driver
   - Configure I2C1 interface

2. **Sensors**
   - Enable LSM6DS3 6-axis IMU

## Running Results

### 1. Terminal Output

After resetting Titan Board Mini, the terminal will output the following information:

![image1](figures/image1.png)

## Related Resources

- [RT-Thread Sensor Framework Documentation](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/sensor/sensor)
