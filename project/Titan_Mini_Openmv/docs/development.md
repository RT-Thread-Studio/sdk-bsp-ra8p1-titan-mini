# 使用 RT-Thread Env 编译

Windows 下使用 [RT-Thread Env](https://github.com/RT-Thread/env-windows) 打开工程，通过 `scons` 完成编译和固件打包。下文命令均在 **Env 的 CMD 终端**中执行。

## 准备环境

构建要求完整 Titan Mini SDK：

```text
sdk-bsp-ra8p1-titan-mini/
├── FSPConfiguration/
│   ├── configuration.xml
│   ├── ra/
│   ├── ra_cfg/
│   └── ra_gen/
├── libraries/
├── rt-thread/
└── project/Titan_Mini_Openmv/
```

| 工具 | 要求 / 用途 |
|---|---|
| RT-Thread Env | 使用 Python 3.10 及以上的 SCons 环境；本工程已验证 Python 3.10 + SCons 4.7.0 |
| Arm GNU Toolchain | 指定 **13.3.Rel1**，用于编译 Cortex-M85 固件 |
| MinGW-w64 GCC、GNU make | Env 终端中可找到 `gcc` 和 `mingw32-make` 或 `make`，用于构建主机工具 `mpy-cross` |
| sh | Env 或 Git for Windows 提供的 `sh.exe`，用于 `mpy-cross` 构建 |

Env 的安装和终端使用方法见 [官方说明](https://github.com/RT-Thread/rt-thread/blob/master/documentation/env/env.md)。本工程的 SCons 文件使用 Python 3 语法，旧版 Python 2 环境不适用。`ThirdParty/` 中已提供固定版本依赖，可直接构建。

## 打开工程并指定工具链

启动 Env 安装目录中的 `env.exe`，进入含有 `SConstruct` 的工程目录，并指定 ARM 工具链路径。将下面的 `{path}` 替换为自己的安装位置。

```bat
cd /d "{path}\sdk-bsp-ra8p1-titan-mini\project\Titan_Mini_Openmv"

set "RTT_EXEC_PATH={path}\ARM\GNU_Tools_for_ARM_Embedded_Processors\13.3\bin"
```

`RTT_EXEC_PATH` 填写 ARM GCC 13.3.Rel1 的 **`bin` 目录**，其中应有 `arm-none-eabi-gcc.exe`，不填写可执行文件本身。该设置仅对当前终端有效，重新打开 Env 后需再次设置。

`rtconfig.py` 已提供 GCC 和 `release` 的默认配置，正常编译无需重复设置。主机 GCC、make 和 sh 由已配置的 Env 环境提供，构建时自动查找。

首次编译前，在同一终端确认工具版本和路径：

```bat
scons --version
"%RTT_EXEC_PATH%\arm-none-eabi-gcc.exe" --version
```

确认 ARM GCC 输出包含 `13.3.Rel1`。若提示找不到 `scons` 或主机工具，检查 Env 环境是否满足上表要求。

## 编译

在同一 Env 终端执行：

```bat
scons -j8
```

`-j8` 表示使用 8 个并行任务，可按电脑配置调整。首次构建会自动生成 QSTR、构建 `mpy-cross`、冻结模块并打包固件；后续直接执行同一命令进行增量编译。更换工具链或切换存储配置后，先清理再编译：

```bat
scons -c
scons -j8
```

## 构建目标

| 命令 | 输出 / 功能 |
|---|---|
| `scons -j8` | 编译、链接、打包，并将 HEX / 视觉文件放入 `firmware/` |
| `scons --prepare-only` | 生成 MicroPython QSTR、模块注册和冻结模块 |
| `scons --pyconfig` | 配置 RT-Thread / BSP |
| `scons -c` | 清除构建目标和 `build/generated/` 中的动态文件 |

存储配置入口：`Titan Mini OpenMV port → OpenMV filesystem and USB MSC medium`。默认 `SD card (SDHI0)`；Flash 选项为 `Onboard SPI Flash (W25Q64, 6 MiB)`。NPU / ML 为固定配置，定义在 `rtconfig_project.h`。

`.config`、`rtconfig.h` 和 `rtconfig_project.h` 共同构成项目配置。配置变化后重新构建并部署配套 HEX / 视觉文件。默认使用 release 编译配置。

## 输出与校验

| 路径 | 用途 |
|---|---|
| `firmware/rtthread.hex` | MCU 片内 MRAM 烧录 |
| `firmware/openmv-vision.bin` | 复制到板端存储根目录 |
| `build/firmware/` | 编译产物，包括 ELF 和 MAP |

`scons` 在链接后自动提取视觉资源、压缩、写入 CRC32 和 SHA256 绑定，并生成 HEX；随后将两个部署文件写入 `firmware/`。无需额外执行 Python 工具。部署时保持两个文件配对，操作见 [上手指南](getting_started.md)。编译和打包成功不代表已完成板端运行验证。

## GitHub CI 与固件下载

仓库提供 [OpenMV Firmware 工作流](../../../.github/workflows/openmv.yml)，使用 Ubuntu、Arm GNU Toolchain 13.3.Rel1 和 SCons 4.7.0 自动编译。CI 分别检查 `sd`、`flash` 两种存储配置；工具链下载后校验固定 SHA-256。

涉及本工程或共用 SDK 源码的 `main` / `master` 推送、Pull Request 会触发构建；也支持每周定时构建和在 Actions 页面手动选择 **Run workflow**。

下载步骤：

1. 打开 GitHub 仓库的 **Actions → OpenMV Firmware**。
2. 选择所需分支或提交的构建记录，确认对应存储配置构建成功。
3. 在运行摘要或 **Artifacts** 中下载 `titan-mini-openmv-sd-<commit>` 或 `titan-mini-openmv-flash-<commit>`。

固件下载包包含 `rtthread.hex`、`openmv-vision.bin`、`SHA256SUMS`、`manifest.json` 和部署说明 `README.txt`。`manifest.json` 记录提交、工具链、存储后端及文件哈希；HEX 和视觉 BIN 必须使用同一下载包中的配对文件。

CI 在上传前核对 HEX 与 ELF、视觉文件内容及固件绑定、实际存储后端。`flash` 包对应板载 W25Q64 文件系统，`sd` 包对应 SD 卡；构建与文件校验不代替板端功能验证。

需要调试时，可下载名称带 `-debug-` 的产物，其中包含已有的 ELF、MAP 和构建日志；构建失败时也会尝试保留诊断文件。产物保留 **30 天**，下载需登录 GitHub 并具备仓库读取权限，操作说明见 [GitHub 官方文档](https://docs.github.com/en/actions/how-tos/manage-workflow-runs/download-workflow-artifacts)。

## 源码与构建逻辑

`platform/port/` 保存已完成板级适配的 C/C++ 源码、头文件和 Kconfig，属于正式源码，修改适配时直接维护这些文件。上游版本及来源见 `ThirdParty/components.lock.json`。一次性的源码转换和补丁生成脚本已删除。

只有依赖当前代码的步骤保留在 SCons 构建流程中：`platform/micropython/SConscript` 生成 QSTR、模块注册及冻结代码；`SConstruct` 按实际对象列表生成视觉链接布局；`board/firmware/SConscript` 负责固件打包。动态产物全部位于 `build/`。适配后的 ML Python 源码位于 `platform/micropython/modules/`，会编译进固件。

SDRAM 视觉分区的图像算法、AprilTag 和 DSP 源文件统一使用 `-O3`，由 `platform/SConscript` 中的 `vision_flags` 管理；显式放入 ITCM 的热点函数保留原有布局。解释器、Python 接口及驱动保持全局 `-Os`，TFLM 保持 `-O2`，CMSIS-NN 保持 `-O3`，不启用 LTO。修改优化选项后，重新构建并配对部署 HEX 与视觉 BIN。

## 示例与模型

直接在 OpenMV IDE 打开 `examples/` 中的脚本，将 `models/sdcard/` 中所需模型和标签保持原文件名复制到板端存储根目录，并按示例说明复制 `assets/` 素材。单手关键点模型及加载路径见 [上手指南](getting_started.md#单手关键点)。示例用法与许可见 [示例说明](../examples/README.md) 和 [模型说明](../models/README.md)。

依赖旧模型目录的导出工具已移除；固件构建和收集不会生成示例或模型压缩包。

## 清理

`scons -c` 清除构建目标。完整清理时，停止构建后删除 `build/` 和 `.sconsign.dblite`，再运行 `scons -j8`。保留 `platform/port/` 和 `platform/micropython/modules/`，它们是正式源码，清理命令不会删除。

烧录及基础功能检查见 [上手指南](getting_started.md)。
