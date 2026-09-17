# 本板硬件与运行约定

本示例集面向 RA8P1 Titan Mini、OV5640 MIPI 摄像头和本工程 OpenMV 5 固件。IDE 显示 AE3 为协议兼容标识。

| 功能 | 接口与限制 |
|---|---|
| LED | `machine.LED(1/2/3)`，也可 `board.LED`，使用数字编号 |
| UART2 | H1/3 TX=P801、H1/2 RX=P802、H1/1 GND；3.3 V TTL，H1 可能未焊接 |
| UART1 | RT-Thread 控制台保留，不用于 Python 示例 |
| I2C0 | U18/27 SDA=P409、/28 SCL=P410，与相机共用 |
| I2C1 | SDA=P511、SCL=P512，板上 LSM6DS3 地址0x6A；不是 U18 I2C |
| SPI1 | U18/23 SCK=P102、/19 MOSI=P708、/21 MISO=P709；手动CS用P105 |
| PWM | GPT6/7/8/12 四组；同组 A/B 互斥，各例已选择独立组 |
| ADC | U18 模拟输入，ADC_B 16位读取，信号限板上3.3 V模拟电源域并共地 |
| 软件总线 | SoftI2C/SoftSPI 按脚本头部接线，不占用板上存储 Flash |
| 定时器 | `Timer(-1)` 软件定时器；回调经VM调度器处理，不是硬实时中断 |
| 存储 | Kconfig 默认SD或可选SPI Flash；`/rom`是根目录只读别名，不是需要新建的文件夹 |

针脚用 MCU 名称如 `P601`，不能使用别的 OpenMV 板的 `P0` 别名或树莓派 GPIO 编号。GPIO 整数编码是 `(port << 8) | pin`。U18/3、/5 是 USBFS P814/P815，不能当 I2C 接线。

所有外设示例都在文件头写明具体接线和条件。电机/舵机需独立供电和共地；PWM脚仅输出信号。脚本示例一次运行一个，停止后重新初始化所用设备。

完整原理图和驱动约束在 SDK 的 `docs/`、项目 `docs/peripherals.md` 和 `docs/storage.md`。
