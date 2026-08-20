# Titan_Mini_rpmsg_master 核间通信示例（M85）

**中文** | [**English**](./README.md)

## 简介

本工程是 Titan Board Mini 双核 RPMSG 通信示例的 Master 端，运行在 Cortex-M85（Core0）。工程上电后初始化 RPMSG 主端，不主动发送消息；通过控制台命令触发，与 Cortex-M33（Core1）上的 `Titan_Mini_rpmsg_remote` 完成一次双向 hello 通信。

配套工程：`project/Titan_Mini_rpmsg/Titan_Mini_rpmsg_remote`。

## 工程结构

- `src/hal_entry.c`：RPMSG Master 初始化、接收处理、`rpmsg_hello` 命令
- `board/linker_scripts/fsp.ld`：Core0 链接脚本，Flash 起始 0x02000000，长度 0xC0000
- `packages/rpmsg-lite-latest`：RPMSG Lite 软件包
- `rt-thread`、`libraries`：链接到仓库根目录的共享代码

## 通信流程

1. M85 初始化 RPMSG Master，等待与 M33 的 RPMSG 通道就绪。
2. 在 M85 控制台输入 `rpmsg_hello`。
3. M85 发送字符串 `hello` 到 M33，并打印 `M85 TX: hello`。
4. M33 收到后打印 `M33 RX: hello`，并自动回发 `hello`。
5. M85 收到回包后打印 `M85 RX: hello`。

## 使用说明

1. 先编译并烧写 `Titan_Mini_rpmsg_remote` 到 Core1 Flash（0x020C0000）。
2. 再编译并烧写本工程到 Core0 Flash（0x02000000）。
3. M85 调试串口为 UART1，M33 调试串口为 UART2。
4. M85 启动后看到 `RPMSG master ready`，在 M85 控制台输入 `rpmsg_hello`。

## 验证输出

M85 侧控制台：

```
RPMSG master ready
M85 TX: hello
M85 RX: hello
```

M33 侧控制台：

```
M33 RX: hello
M33 TX: hello
```
