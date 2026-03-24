# IMU Sensor Example

**English** | [**中文**](README_zh.md)

## Introduction

This example demonstrates how to use the IMU (Inertial Measurement Unit) sensor on the **Titan Board Mini** with **RT-Thread's sensor framework**.

Main features include:
- Initialization of IMU sensor (accelerometer, gyroscope)
- Reading 6-axis motion data
- Integration with RT-Thread sensor framework

## IMU Introduction

### 1. Overview

**IMU (Inertial Measurement Unit)** typically consists of an accelerometer and gyroscope, used to measure acceleration and angular velocity.

### 2. Common IMU Sensors

- **MPU6050**: 6-axis IMU (3-axis gyro + 3-axis accel)
- **BMI160**: High-performance 6-axis IMU
- **ICM42688**: High-precision 6-axis IMU

### 3. RT-Thread Sensor Framework

RT-Thread provides sensor device framework:
- `rt_device_find()` - Find sensor device
- `rt_device_open()` - Open sensor device
- `rt_device_read()` - Read sensor data

## Hardware Description

IMU sensor is connected via I2C or SPI. Please refer to the hardware manual for pin connections.

## Software Description

The source code is located in `src/hal_entry.c`.

## Compilation & Download

1. Open the project in RT-Thread Studio
2. Build the project
3. Connect the debug probe
4. Download the firmware

## Run Effect

The terminal will display IMU sensor data (acceleration and gyroscope values).

## Further Reading

- [RT-Thread Sensor](https://www.rt-thread.org/document/site/)
