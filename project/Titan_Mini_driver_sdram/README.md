# SDRAM Driver Example

**English** | [**中文**](README_zh.md)

## Introduction

This example demonstrates how to use the SDRAM peripheral on the **Titan Board Mini** with **RT-Thread's memory device framework**.

Main features include:
- Initialization of RA8P1 SDRAM controller
- Memory configuration and timing setup
- Integration with RT-Thread's memory management

## SDRAM Introduction

### 1. Overview

**SDRAM (Synchronous Dynamic Random Access Memory)** is a type of volatile memory that synchronizes with the system clock for faster data access.

### 2. RA8P1 SDRAM Features

- **High capacity**: Up to 64MB external memory
- **High speed**: Supports high-frequency operation
- **32-bit bus**: Wide data bus for high throughput

## Hardware Description

SDRAM chip is connected to the MCU's external memory interface. Please refer to the hardware manual for details.

## Software Description

The source code is located in `src/hal_entry.c`.

## Compilation & Download

1. Open the project in RT-Thread Studio
2. Build the project
3. Connect the debug probe
4. Download the firmware

## Run Effect

The terminal will display SDRAM initialization status and memory test results.
