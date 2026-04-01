# SPI Driver Example Guide

**English** | [**中文**](README_zh.md)

## Project Introduction

This example demonstrates how to use the **RT-Thread SPI Device Driver Framework** on the **Titan Board Mini** development board for Serial Peripheral Interface communication. Based on the hardware SPI peripheral of the RA8P1 microcontroller, it supports multiple communication modes and high-speed data transfer.

### Key Features
- RA8P1 hardware SPI initialization and configuration
- SPI master mode communication
- DMA-supported high-speed transfer
- Support for multiple SPI device integration

## Directory Structure

```
Titan_Mini_driver_spi/
├── src/
│   └── hal_entry.c          # Main SPI driver code
├── ra/
│   ├── fsp/src/r_spi_b/     # SPI low-level driver implementation
│   └── hal_data.c          # Hardware abstraction layer configuration
├── ra_gen/
│   └── hal_data.c          # Generated hardware configuration
├── libraries/
│   └── M85_Config/
└── README_zh.md            # Chinese documentation
```

## 1. Hardware Introduction - RA8P1 SPI Features

### 1.1 RA8P1 Microcontroller Overview
RA8P1 is a 32-bit ARM Cortex-M33 microcontroller from Renesas RA series, designed for embedded applications with powerful peripheral interfaces and low-power features.

### 1.2 SPI Hardware Features

#### 1.2.1 Multi-Channel Support
- **SPI0**: Supports 2 channels
- **SPI1**: Supports 2 channels
- **SPI2**: Supports 2 channels
- **SPI3**: Supports 1 channel
- **SPI4**: Supports 1 channel
- **OSPI**: 1 OctoSPI channel

#### 1.2.2 High-Speed Transfer Features
- **Maximum Clock Frequency**: 25 MHz (depends on system configuration)
- **DMA Support**: Bidirectional DMA transfer, reducing CPU burden
- **Data Width**: Supports 1-16 bit configurable data width
- **Transfer Mode**: Supports master mode and slave mode

#### 1.2.3 Clock Configuration Features
- **Clock Polarity (CPOL)**: Configurable high or low idle level
- **Clock Phase (CPHA)**: Configurable edge trigger mode
- **Clock Divider**: Supports 2-4096 division
- **Clock Source**: Selectable from multiple clock sources

#### 1.2.4 Chip Select Control
- **Multi-CS Support**: Supports multiple slave devices
- **Software Control**: Chip select signal can be controlled by software
- **Hardware Control**: Automatic chip select signal management
- **Polarity Configuration**: Supports active-high or active-low

### 1.3 SPI Pin Mapping

#### Titan Board Mini SPI1 Pin Configuration
```
SPI1 Pin Definition:
- MOSI (Master Out Slave In):  PB_15
- MISO (Master In Slave Out):  PB_14
- SCLK (Clock):      PB_13
- SS (Chip Select):  PB_12
```

**Note**: Actual pin configuration may vary according to project settings. Please refer to specific hardware connections.

## 2. Software Architecture - RT-Thread SPI Device Framework

### 2.1 RT-Thread SPI Architecture Overview

RT-Thread provides a complete SPI device driver framework with a layered architecture design:

```
Application Layer
    ↓
SPI Device Layer (rt_spi_device)
    ↓
SPI Bus Layer (rt_spi_bus)
    ↓
SPI Operation Layer (rt_spi_ops)
    ↓
Hardware Abstraction Layer (HAL)
```

### 2.2 Main Data Structures

#### 2.2.1 SPI Configuration Structure
```c
struct rt_spi_configuration {
    rt_uint8_t mode;          /* Operation mode */
    rt_uint8_t data_width;    /* Data bit width */
    rt_uint16_t reserved;     /* Reserved field */
    rt_uint32_t max_hz;       /* Maximum transfer frequency */
};
```

#### 2.2.2 SPI Device Structure
```c
struct rt_spi_device {
    struct rt_device parent;          /* Parent device */
    struct rt_spi_bus *bus;          /* Bus */
    struct rt_spi_configuration config; /* Configuration */
    rt_base_t cs_pin;                /* Chip select pin */
    void *user_data;                 /* User data */
};
```

### 2.3 SPI Operation Modes

#### 2.3.1 Master/Slave Modes
- **Master Mode (RT_SPI_MASTER)**: Initiates data transfer, controls clock signal
- **Slave Mode (RT_SPI_SLAVE)**: Responds to master commands, receives clock signal

#### 2.3.2 Clock Modes
```c
#define RT_SPI_MODE_0   (0 | 0)        /* CPOL=0, CPHA=0 */
#define RT_SPI_MODE_1   (0 | RT_SPI_CPHA)   /* CPOL=0, CPHA=1 */
#define RT_SPI_MODE_2   (RT_SPI_CPOL | 0)   /* CPOL=1, CPHA=0 */
#define RT_SPI_MODE_3   (RT_SPI_CPOL | RT_SPI_CPHA) /* CPOL=1, CPHA=1 */
```

#### 2.3.3 Data Transfer Modes
- **MSB First**: Data transfer starts from the most significant bit
- **LSB First**: Data transfer starts from the least significant bit

### 2.4 Core API Interfaces

#### 2.4.1 Device Find
```c
struct rt_spi_device *rt_device_find(const char *name);
```

#### 2.4.2 Device Configure
```c
rt_err_t rt_spi_configure(struct rt_spi_device *device,
                          struct rt_spi_configuration *cfg);
```

#### 2.4.3 Data Transfer
```c
rt_size_t rt_spi_transfer(struct rt_spi_device *device,
                          const void *send_buf,
                          void *recv_buf,
                          rt_size_t length);
```

#### 2.4.4 Device Attach
```c
rt_err_t rt_hw_spi_device_attach(const char *bus_name,
                                 const char *device_name,
                                 void *user_data);
```

## 3. Running Effect Example

Use dupont wires to short the P708 and P709 pins of the Raspberry Pi expansion interface on the development board (connection location shown in the figure below) to build a loopback test, as shown in the figure below

![alt text](figures/image2.png)

After completing the hardware connection, compile the project and flash the firmware to the development board. Open the serial terminal and enter the command `spi_loop_test` to observe the test results

![alt text](figures/image1.png)

---

**Reference Documents**:
- [RT-Thread SPI Device Documentation](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/spi/spi)
