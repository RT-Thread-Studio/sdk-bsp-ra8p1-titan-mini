# Template Project

**English** | [**中文**](README_zh.md)

## Introduction

This is a template project for **Titan Board Mini**. It implements basic LED blinking functionality and can be used as a starting point for secondary development.

## Features

This template provides:
- RT-Thread full system configuration
- Basic LED driver (GPIO)
- Serial debug output (UART)
- Project structure for secondary development

## Project Structure

```
project/
├── board/              # Board configuration
├── libraries/          # RA8P1 HAL libraries
├── rt-thread/          # RT-Thread kernel
├── src/                # Application source code
├── Kconfig             # Menuconfig configuration
├── SConstruct          # Build script
└── ...
```

## Quick Start

1. Open the project in RT-Thread Studio
2. Configure the project using menuconfig if needed
3. Build the project
4. Download and run

## Secondary Development

This template can be used as a base for:
- Adding new peripherals
- Implementing custom applications
- Developing drivers
- Building complete applications

## Further Reading

- [RT-Thread Documentation](https://www.rt-thread.org/document/site/)
