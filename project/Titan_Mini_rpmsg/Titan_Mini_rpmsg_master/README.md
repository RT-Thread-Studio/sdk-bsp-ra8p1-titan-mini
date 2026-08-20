# Titan_Mini_rpmsg_master Inter-Core Communication Example (M85)

**English** | [**Chinese**](./README_zh.md)

## Introduction

This project is the Master side of the Titan Board Mini dual-core RPMSG communication example and runs on Cortex-M85 (Core0). It initializes the RPMSG master after power-on and does not send messages automatically. The communication is triggered by a console command, exchanging a bidirectional hello with `Titan_Mini_rpmsg_remote` on Cortex-M33 (Core1).

Companion project: `project/Titan_Mini_rpmsg/Titan_Mini_rpmsg_remote`.

## Project Structure

- `src/hal_entry.c`: RPMSG master initialization, receive handling, and the `rpmsg_hello` command
- `board/linker_scripts/fsp.ld`: Core0 linker script, Flash starts at 0x02000000 with length 0xC0000
- `packages/rpmsg-lite-latest`: RPMSG Lite package
- `rt-thread`, `libraries`: shared code linked from the repository root

## Communication Flow

1. M85 initializes the RPMSG master and waits for the RPMSG channel with M33 to be ready.
2. Enter `rpmsg_hello` on the M85 console.
3. M85 sends the string `hello` to M33 and prints `M85 TX: hello`.
4. M33 receives it, prints `M33 RX: hello`, and automatically replies with `hello`.
5. M85 receives the reply and prints `M85 RX: hello`.

## Usage

1. Build and flash `Titan_Mini_rpmsg_remote` to Core1 Flash (0x020C0000) first.
2. Build and flash this project to Core0 Flash (0x02000000).
3. The M85 debug console uses UART1; the M33 debug console uses UART2.
4. After M85 prints `RPMSG master ready`, enter `rpmsg_hello` on the M85 console.

## Expected Output

M85 console:

```
RPMSG master ready
M85 TX: hello
M85 RX: hello
```

M33 console:

```
M33 RX: hello
M33 TX: hello
```
