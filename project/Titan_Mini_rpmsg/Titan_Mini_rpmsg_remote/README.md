# Titan_Mini_rpmsg_remote Inter-Core Communication Example (M33)

**English** | [**Chinese**](./README_zh.md)

## Introduction

This project is the Remote side of the Titan Board Mini dual-core RPMSG communication example and runs on Cortex-M33 (Core1). It is started by M85, initializes the RPMSG remote endpoint, and waits for incoming messages. When it receives `hello` from M85, it automatically replies with `hello`.

Companion project: `project/Titan_Mini_rpmsg/Titan_Mini_rpmsg_master`.

## Project Structure

- `src/hal_entry.c`: RPMSG remote initialization, receive handling, and automatic reply logic
- `board/linker_scripts/fsp.ld`: Core1 linker script, Flash starts at 0x020C0000 with length 0x30000
- `packages/rpmsg-lite-latest`: RPMSG Lite package
- `rt-thread`, `libraries`: shared code linked from the repository root

## Communication Flow

1. M33 initializes the RPMSG remote and waits for the RPMSG channel with M85 to be ready.
2. After `rpmsg_hello` is entered on the M85 console, M85 sends `hello` to M33.
3. M33 receives it, prints `M33 RX: hello`, and automatically replies with `hello`.
4. M33 prints `M33 TX: hello` after replying, and M85 prints `M85 RX: hello` after receiving it.

## Usage

1. Build and flash this project to Core1 Flash (0x020C0000) first.
2. Build and flash `Titan_Mini_rpmsg_master` to Core0 Flash (0x02000000).
3. The M33 debug console uses UART2; the M85 debug console uses UART1.
4. After resetting the board, enter `rpmsg_hello` on the M85 console to verify bidirectional communication.

## Expected Output

M33 console:

```
M33 RX: hello
M33 TX: hello
```

M85 console:

```
RPMSG master ready
M85 TX: hello
M85 RX: hello
```
