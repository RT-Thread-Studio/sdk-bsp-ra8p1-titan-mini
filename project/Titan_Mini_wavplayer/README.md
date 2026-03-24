# WAV Audio Player Example

**English** | [**中文**](README_zh.md)

## Introduction

This example demonstrates how to play WAV audio files on the **Titan Board Mini** using RT-Thread's audio framework.

Main features include:
- SD card file system integration
- WAV audio file decoding
- I2S audio output
- Audio playback control

## Audio Introduction

### 1. Overview

WAV is an audio file format that stores audio data in uncompressed PCM format. It provides high-quality audio playback.

### 2. Audio System

- **I2S interface**: For digital audio transmission
- **Audio codec**: For DAC conversion
- **File system**: For reading audio files from SD card

### 3. RT-Thread Audio Framework

RT-Thread provides audio device framework:
- `rt_device_find()` - Find audio device
- `rt_device_open()` - Open audio device
- `rt_device_write()` - Play audio data

## Hardware Description

Audio output is through I2S interface to audio codec. Please refer to the hardware manual for pin connections.

## Software Description

The source code is located in `src/hal_entry.c`.

## Compilation & Download

1. Open the project in RT-Thread Studio
2. Build the project
3. Connect the debug probe
4. Download the firmware

## Run Effect

The terminal will display audio playback status and controls.

## Further Reading

- [RT-Thread Audio](https://www.rt-thread.org/document/site/)