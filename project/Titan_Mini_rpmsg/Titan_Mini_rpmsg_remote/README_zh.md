# Titan_Mini_rpmsg_remote 核间通信示例（M33）

**中文** | [**English**](./README.md)

## 简介

本工程是 Titan Board Mini 双核 RPMSG 通信示例的 Remote 端，运行在 Cortex-M33（Core1）。工程由 M85 启动，初始化 RPMSG 从端后等待接收消息；收到 M85 发来的 `hello` 后自动回发 `hello`。

配套工程：`project/Titan_Mini_rpmsg/Titan_Mini_rpmsg_master`。

## 工程结构

- `src/hal_entry.c`：RPMSG Remote 初始化、接收处理、自动回包逻辑
- `board/linker_scripts/fsp.ld`：Core1 链接脚本，Flash 起始 0x020C0000，长度 0x30000
- `packages/rpmsg-lite-latest`：RPMSG Lite 软件包
- `rt-thread`、`libraries`：链接到仓库根目录的共享代码

## 通信流程

1. M33 初始化 RPMSG Remote，等待 M85 的 RPMSG 通道就绪。
2. M85 控制台执行 `rpmsg_hello` 后，向 M33 发送 `hello`。
3. M33 收到后打印 `M33 RX: hello`，并自动回发 `hello`。
4. M33 回包后打印 `M33 TX: hello`，M85 收到后打印 `M85 RX: hello`。

## 使用说明

1. 先编译并烧写本工程到 Core1 Flash（0x020C0000）。
2. 再编译并烧写 `Titan_Mini_rpmsg_master` 到 Core0 Flash（0x02000000）。
3. M33 调试串口为 UART2，M85 调试串口为 UART1。
4. 复位开发板后，在 M85 控制台输入 `rpmsg_hello` 验证双向通信。

## 验证输出

M33 侧控制台：

```
M33 RX: hello
M33 TX: hello
```

M85 侧控制台：

```
RPMSG master ready
M85 TX: hello
M85 RX: hello
```
