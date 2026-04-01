# CAN FD Driver Example Guide

[**Chinese**](README_zh.md) | **English**

## Introduction

This document provides a detailed introduction to the **CAN FD (Controller Area Network with Flexible Data-Rate)** driver implemented on the **Titan Board Mini** development board based on the **RA8P1** microcontroller. CAN FD is an enhanced version of the traditional CAN protocol, providing higher data transmission rates and larger data payload capacity while maintaining backward compatibility.

### Key Features

- **High-performance CAN FD communication**: Supports data transmission rates up to 8 Mbps
- **Large data transfer**: Supports up to 64 bytes of data per frame (traditional CAN only supports 8 bytes)
- **Dual-rate mode**: Different baud rates can be configured for arbitration phase and data phase
- **RT-Thread unified driver framework**: Standardized device operation interface
- **Interrupt-driven transceiver mechanism**: Efficient asynchronous communication handling
- **Comprehensive error detection**: Enhanced error detection and handling mechanisms

---

### RA8P1 CAN FD Hardware Features

**RA8P1** is a high-performance microcontroller launched by Renesas Electronics, based on the **Arm Cortex-M85** core, featuring powerful CAN FD peripheral capabilities:

#### Core Features
- **Processor Architecture**: Arm Cortex-M85 @ 1GHz (RA8P1) + Arm Ethos-U55 @ 500MHz (NPU)
- **Operating Frequency**: 800MHz / 1GHz selectable
- **Memory Configuration**: 2MB SRAM (ECC protected), 512KB MRAM, 4-8MB Flash options

#### CAN FD Peripheral Specifications
- **Number of CAN FD Channels**: 2 independent CAN FD channels
- **Maximum Arbitration Baud Rate**: 1 Mbps
- **Maximum Data Baud Rate**: **8 Mbps** (industry-leading level)
- **Filter Rules**: 16 per channel
- **Transmit Message Buffers**: **16 per channel**
- **Receive Buffers**: **16 per channel**
- **Receive FIFOs**: 2 per channel
- **Receive Buffer RAM**: 1216 bytes
- **General FIFO**: 1 per channel
- **64-byte Storage**: 16 messages

#### Clock and Timer
- **CAN Clock Frequency**: 40MHz (recommended configuration)
- **Bit Time Precision**: High-precision bit time configuration
- **Sampling Point Configuration**: Programmable sampling point (50%-87.5%)
- **Synchronization Jump Width**: 1-128 time quanta

---

## Software Architecture - RT-Thread CAN Device Framework

### RT-Thread Driver Model

RT-Thread provides a unified I/O device management framework, and CAN devices are integrated into the system as one class of devices (`RT_Device_Class_CAN`).

#### Device Management Three-Layer Architecture

```
┌─────────────────────────┐
│    Application Layer    │
├─────────────────────────┤
│ I/O Device Management   │
│      Layer              │
├─────────────────────────┤
│   Device Driver Layer   │
└─────────────────────────┘
```

#### Core Data Structures

```c
struct rt_can_device {
    struct rt_device parent;           /* Device base class */
    const struct rt_can_ops *ops;      /* Device operation interface */
    struct can_configure config;       /* Device configuration */
    struct rt_can_status status;       /* Device status */
    rt_uint32_t timer_init_flag;       /* Timer initialization flag */
    struct rt_timer timer;            /* Timer */
    struct rt_can_status_ind_type status_indicate; /* Status indication */
    struct rt_mutex lock;             /* Mutex lock */
    void *can_rx;                     /* Receive buffer */
    void *can_tx;                     /* Transmit buffer */
};
```

### CAN Device Operation Interface

The RT-Thread CAN device framework provides standardized operation interfaces:

#### Device Search and Initialization
```c
rt_device_t rt_device_find(const char *name);      // Find device
rt_err_t rt_device_open(rt_device_t dev, rt_uint16_t oflag); // Open device
```

#### Data Read/Write Operations
```c
rt_size_t rt_device_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size);  // Read data
rt_size_t rt_device_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size); // Write data
```

#### Interrupt and Callback Configuration
```c
rt_err_t rt_device_set_rx_indicate(rt_device_t dev, rt_err_t(*rx_ind)(rt_device_t dev, rt_size_t size)); // Set receive callback
rt_err_t rt_device_set_tx_complete(rt_device_t dev, rt_err_t(*tx_complete)(rt_device_t dev, void *buffer)); // Set transmit complete callback
```

### Interrupt Handling Mechanism

CAN FD adopts an interrupt-driven communication mechanism, mainly including:

#### Receive Interrupt Handling
1. **Hardware Interrupt Trigger**: CAN receives data frame
2. **Interrupt Service Routine**: Read hardware status register
3. **Data Transfer**: Copy data from hardware buffer to application buffer
4. **Callback Notification**: Notify receive thread through semaphore
5. **Thread Processing**: Receive thread processes data and application logic

#### Transmit Interrupt Handling
1. **Transmit Request**: Application calls transmit interface
2. **Buffer Check**: Check transmit buffer availability
3. **Data Write**: Write data to transmit buffer
4. **Start Transmission**: Trigger hardware transmission process
5. **Transmission Complete**: Trigger interrupt after transmission completion

#### Error Interrupt Handling
CAN FD supports multiple error detection mechanisms:
- **Bit Error Detection**: Detect inconsistency between transmitted and received bits
- **Stuffing Error**: Detect bit stuffing errors
- **CRC Error**: Use enhanced 17-bit or 21-bit CRC
- **Format Error**: Detect frame format errors
- **Overload Error**: Handle bus overload conditions

---

## Operation Example

This section introduces how to use the **PCAN-View** tool to perform CAN FD communication testing with Titan Board Mini.

### 1. Hardware Connection

#### 1.1 Connect CAN Interface

Connect the CAN interface of Titan Board Mini to the PCAN adapter:

![CAN Interface Location](figures/image1.png)

> **Note**: Please ensure correct wiring. CAN_H and CAN_L must not be reversed, otherwise communication will fail.

### 2. PCAN-View Software Configuration

#### 2.1 Launch PCAN-View

Open the PCAN-View software and configure the connection:

![PCAN-View Connection Configuration](figures/20da2ff61cd8289eceb96587f42440fd.png)

**Configuration Steps**:
1. Select PCAN adapter type (e.g., PCAN-USB FD)
2. Select CAN channel (Channel 1 or Channel 2)
3. Set baud rate
4. Select CAN FD mode
5. Click the "ok" button to establish connection

#### 2.2 PCAN-View Display Configuration

Configure display options in PCAN-View:

- **Display Format**: Hex (hexadecimal)
- **Timestamp**: Enable
- **Frame Type**: Display both standard and extended frames

### 3. Terminal Command Operations

#### 3.1 Start Board Program

Reset the development board in the terminal, the program will automatically run and start sending CAN FD messages.

![Terminal Output](figures/image2.png)

#### 3.2 Start CAN Test Program

Run the test program to start sending CAN FD messages:

### 4. Data Transceiver Testing

#### 4.1 Receive Data Sent from Board

After the program runs, PCAN-View will receive CAN FD messages sent by Titan Board Mini:

![Received Messages](figures/image3.png)


#### 4.2 Send Data from PCAN-View

Send CAN FD messages to the board through PCAN-View:

**Configure Transmit Parameters**:
1. Right-click in the "Transmit" section of PCAN-View to pop up New Message
2. Configure message parameters:
   - **ID**: 0x78
   - **Frame Type**: CAN FD
   - **Data Length**: 8 bytes
   - **Data**: Custom data content

![Transmit Configuration](figures/b65b5eb894ec2bbefa80ece22bed1122.png)

![Send Data](figures/e5a3c7971c18a6e098e254b47dc22f85.png)

#### 4.3 View Received Data in Terminal

After the board receives the message sent by PCAN-View, the terminal will display the received data:

![Final Result](figures/image4.png)


