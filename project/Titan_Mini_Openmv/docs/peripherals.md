# 外设接口与接线

`machine` 模块使用 MCU 引脚名，例如 `Pin("P601")`。整数编码为 `(port << 8) | pin`，P715 对应 `0x070f`。U18 物理脚号用于接线，不作为 `Pin` 参数。

硬件资料：[原理图](../../../docs/Titan_Mini_schematic_v1.0.pdf) 第 3、8、13 页，[芯片手册](../../../docs/Titan_Mini_datasheet.pdf)。应用接线见 [外设示例](../examples/03_外设/README.md) 文件头。

## 接口

| 外设 | 能力与限制 |
|---|---|
| LED | `machine.LED(1)`、`LED(2)`、`LED(3)`，支持 `on/off/toggle` |
| GPIO | 输入、输出、开漏、上拉，支持上升沿 / 下降沿 / 双边沿 IRQ；不支持内部下拉 |
| PWM | GPT6 / 7 / 8 / 12 四组，每组同时一个输出，最多四路 |
| ADC | U18 十路模拟输入，ADC_B 单次采样，16 位原始读数 |
| UART | H1 UART2；UART1 保留给 RT-Thread 控制台 |
| I2C | I2C0 / 1 固定板级配置；可用 GPIO 支持 `SoftI2C` |
| SPI | SPI1 主机，软件片选；可用 GPIO 支持 `SoftSPI` |
| Timer | 8 个软件定时器槽；`Timer(-1)` 自动分配，`Timer(0)`..`Timer(7)` 指定槽；周期参数为 `period`，单位 ms |

GPIO IRQ 和软件定时器回调由 VM 调度器执行，延迟受 Python 执行和其他任务影响。未实现的接口：UART Python IRQ、UART 硬件流控、I2C / SPI 从机、RTC、WDT、CAN、DAC、I2S、USB HID。

## U18 与 H1 接线

| U18 物理脚 | MCU 引脚 | 可用功能 |
|---|---|---|
| 7 | P601 | GPIO、GPT6A |
| 11 | P000 | GPIO、AN000 |
| 12 | P604 | GPIO、GPT8B |
| 13 | P001 | GPIO、AN001 |
| 15 | P002 | GPIO、AN002 |
| 16 | P004 | GPIO、AN004 |
| 18 | P005 | GPIO、AN005 |
| 22 | P006 | GPIO、AN006 |
| 29 | P008 | GPIO、AN008 |
| 31 | P009 | GPIO、AN009 |
| 32 | P605 | GPIO、GPT8A |
| 33 | P603 | GPIO、GPT7A |
| 35 | P602 | GPIO、GPT7B |
| 36 | P714 | GPIO、GPT12B |
| 37 | P014 | GPIO、AN014 |
| 38 | P715 | GPIO、GPT12A |
| 40 | P015 | GPIO、AN015 |

| 接口 | 接线 | 共享关系 |
|---|---|---|
| UART1 | U18/8 TX=P707、U18/10 RX=P706 | 调试控制台专用，`UART(1)` 拒绝占用 |
| UART2 | H1/3 TX=P801、H1/2 RX=P802、H1/1 GND | 3.3 V TTL；H1 可能未焊排针 |
| SPI1 | U18/23 SCK=P102、/19 MOSI=P708、/21 MISO=P709 | 也连接相机 BTB，各外接设备必须独立片选 |
| SPI1 片选 | U18/24 P105 或 /26 P106 | Python `Pin` 手动控制；P106 还有 GPT8B 复用 |
| I2C0 | U18/27 SDA=P409、/28 SCL=P410 | 与 OV5640 和 LCD 触摸共用 |
| I2C1 | SDA=P511、SCL=P512，未接 U18 | 板载 LSM6DS3（0x6A）、ES8156（0x08） |

U18/3、U18/5 分别连接 USBFS 的 P814、P815。外接设备与开发板共地；ADC 输入位于 3.3 V 模拟电源域。电机、舵机使用独立电源，PWM 引脚连接控制输入。

## 调用示例

以下代码段独立运行。

```python
from machine import Pin
p = Pin("P601", Pin.OUT, value=0)
p.on()
p.off()
```

```python
from machine import Pin, PWM
pwm = PWM(Pin("P601"), freq=1000, duty_u16=32768)
# 1 kHz、约 50% 占空比
pwm.deinit()
```

```python
from machine import Pin, ADC
adc = ADC(Pin("P000"))
print(adc.read_u16())  # 0..65535，原始值，不是电压
adc.deinit()
```

```python
from machine import UART
uart = UART(2, 115200, bits=8, parity=None, stop=1,
            timeout=100, timeout_char=10, rxbuf=256)
uart.write(b"Titan Mini\r\n")
uart.flush()
print(uart.read(16))
uart.deinit()
```

```python
from machine import I2C
i2c = I2C(1)
print([hex(addr) for addr in i2c.scan()])
print(i2c.readfrom_mem(0x6A, 0x0F, 1))  # LSM6DS3 WHO_AM_I
```

SPI、SoftSPI 和 SoftI2C 的器件接线与事务示例见 [SPI](../examples/03_外设/04_SPI/) 和 [I2C](../examples/03_外设/03_I2C/)。

## 参数与返回值

| 接口 | 约束 |
|---|---|
| PWM | `freq`：1..1,000,000 Hz，周期计数取整；`duty_u16`：0..65535；`duty`：0..255；`duty_ns`：脉宽 ns，不能超过周期。一次指定一种占空比格式。0% / 100% 为固定低 / 高输出 |
| ADC | `read()`、`read_u16()` 返回 0..65535 原始值；`sample_ns` 为非负采样时间，至少使用 95 个采样时钟。首次使用及切换通道 / 采样配置时校准，转换串行执行 |
| UART2 | `bits`：7 / 8；`parity`：`None` / `0`（偶）/ `1`（奇）；`stop`：1 / 2；`rxbuf`：1..32766 字节；`timeout`、`timeout_char`：非负 ms。无数据时 `read()` 返回 `None`，`readchar()` 返回 `-1`；接收缓冲满时丢弃新数据，发送超时可能短写 |
| I2C0 / 1 | 固定 `freq=400000`、`timeout=100000`（µs），引脚固定；板级配置的实际时钟约 393 kHz。寄存器读取使用总线锁保护的重复起始事务，不支持独立 `stop=False` |
| SPI1 | `polarity`、`phase`：0 / 1；`firstbit`：`SPI.MSB` / `SPI.LSB`；`bits`：8 / 16 / 32。缓冲长度为字宽整数倍，按 Cortex-M 小端布局解释；软件片选由 `Pin` 控制 |
| Timer | `period`：正整数 ms，默认 1000；`mode`：`Timer.PERIODIC` / `Timer.ONE_SHOT`；`callback` 接收定时器对象。`deinit()` 释放槽 |

SoftI2C 用于其他时钟和事务配置，需连接外部上拉并选用空闲 GPIO。

## 资源占用与释放

- PWM 的 P603/P602、P605/P604、P715/P714 分别共用 GPT7、GPT8、GPT12，组内互斥；`deinit()` 释放定时器并恢复引脚配置。
- ADC 同一引脚只允许一个活动对象。
- GPIO IRQ 同通道只允许一个引脚：P008/P715/P801 共用 IRQ12；P009/P015/P714 共用 IRQ13；P014/P603 共用 IRQ27。`pin.irq(handler=None)` 释放 IRQ。
- UART、SPI、PWM、ADC 和 IRQ 在 Python 软复位时释放。硬件 I2C 由板级持有，软复位保持总线运行。
- 存储、USB、SDRAM、相机、控制台和硬件 I2C 引脚由固件保留，GPIO / 软件总线不能重新占用。

## 故障处理

| 现象 | 处理 |
|---|---|
| `EBUSY` 或引脚占用错误 | 对原对象执行 `deinit()`，IRQ 使用 `irq(handler=None)`；检查同组 GPT 和共享 IRQ |
| I2C 配置报 `NotImplementedError` | 使用固定 400 kHz / 100 ms 参数，或改用空闲引脚的 SoftI2C |
| SPI 超时后对象不可用 | 检查片选、时钟和接线后执行 `spi.init()` |
| 停止脚本后外设状态残留 | 执行软复位并重新初始化；硬件异常时执行硬件复位 |

接口实现位于 [platform/machine](../platform/machine/)，IRQ 映射见 [titan_pin_irq.c](../platform/drivers/titan_pin_irq.c)。
