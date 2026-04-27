# LED Blink Example

**English** | [**中文**](README_zh.md)

## Introduction

This example is the simplest and most basic example in the SDK, similar to the "Hello World" program that programmers encounter as their first program. The main function of this example is to make the onboard RGB LED blink periodically.

## GPIO Introduction

### 1. Overview

**GPIO (General Purpose Input/Output)** is one of the most commonly used peripheral interfaces of an MCU. It can be configured under software control as either **input mode** or **output mode**:

- **Input mode**: Used to read external voltage levels, such as button inputs.
- **Output mode**: Used to control peripheral voltage levels, such as lighting up an LED or driving a buzzer.

### 2. RA8 Series GPIO Features

- **Rich pin resources**: Each port can be independently configured as input/output/multiplexed function.
- **Multi-function multiplexing**: In addition to GPIO functionality, the same pin can also serve as a signal line for peripherals such as UART, SPI, or I2C.
- **Level control**: In output mode, pins can drive either high or low levels; some pins support open-drain mode.
- **Direction control**: Input/output direction can be dynamically switched.

### 3. RT-Thread PIN Framework

RT-Thread provides a **PIN device driver framework**, which offers a unified interface to shield hardware differences:

- `rt_pin_mode()` - Set the working mode of a pin (input/output/pull-up/pull-down, etc.)
- `rt_pin_write()` - Output a voltage level (high/low)
- `rt_pin_read()` - Read an input voltage level

This way, developers don't need to directly manipulate registers; GPIO control can be achieved through RT-Thread's API.

In this example, the LED pin is configured as **output mode**, and the software continuously toggles between high and low levels to make the LED blink.

## Hardware Description

Titan Board Mini provides three user LEDs: LED_R (RED), LED_B (BLUE), LED_G (GREEN). The LED corresponds to specific pins on the MCU. To turn on the LED, a low level signal is output from the MCU pin, and to turn it off, a high level signal is output.

## Software Description

The source code for this example is located in `src/hal_entry.c`. The MCU pin definitions for the LEDs and the LED control logic can be found in the source file.

```c
/* Configure LED pins */
rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

/* Control LED */
rt_pin_write(LED_PIN, LED_ON);  // Turn on
rt_pin_write(LED_PIN, LED_OFF); // Turn off
```

## Compilation & Download

1. Open the project in RT-Thread Studio
2. Build the project using the build button
3. Connect the debug probe to the development board
4. Download the firmware to the board

## Run Effect

After pressing the reset button to restart the development board, observe the LED blinking effect. The RGB LEDs will change periodically.

## Further Reading

- [RT-Thread PIN Device](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/pin/pin)