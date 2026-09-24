# Titan Mini 外设示例

每个脚本独立运行，接线、电平、占用条件写在文件头。路径参考 RA8D1 示例集，实际 API 和引脚依据本工程 [本板硬件约定](../BOARD.md) 与驱动实现。不要使用其他 OpenMV 板的 P0/P1、LED_BLUE 或 UART1 配置。

| 分类 | 内容 | 前提 |
|---|---|---|
| 01_GPIO与LED | 闪烁、组合灯、GPIO 输入输出、中断、开漏、Signal、脉宽测量 | LED 无需接线；其余按脚本连接 U18 |
| 02_UART | UART2 发送、回显、回环 | H1 3.3 V TTL，UART1 保留给 msh |
| 03_I2C | I2C0/1 扫描、板载 IMU 身份、外接 EEPROM、SoftI2C | 硬件总线固定 400 kHz；软件总线独立引脚和外接上拉 |
| 04_SPI | SPI1 回环、SoftSPI 回环、外接 Flash ID | 先按各脚本接线；不访问板载存储 Flash |
| 05_PWM | 占空比、脉宽/舵机信号、四路输出 | GPT6/7/8/12 每组只允许一个输出 |
| 06_ADC | 单/双路 ADC_B 16 位采样 | U18 模拟信号在 0..3.3 V 模拟电源域内 |
| [07_LCD](07_LCD) | 基础绘图与背光、相机预览、色块与 YOLO 姿态识别显示 | 800×480 RGB565 屏，含 `lcd/display` 的固件及配套视觉 BIN；相机示例需 OV5640，姿态示例另需模型 |

LCD 示例按 [基础显示](07_LCD/lcd_basic.py)、[相机预览](07_LCD/lcd_camera.py)、[色块识别显示](07_LCD/lcd_color_tracking.py) 的顺序运行。相机图像以 2 倍等比缩放居中显示，保留左右黑边；识别示例的 LAB 阈值需按现场光照调整。脚本退出会关闭 LCD，并恢复 IDE 预览；LCD 占用 GPT7，不与同组 PWM 示例同时运行。

[YOLO 姿态识别与 LCD 显示](07_LCD/lcd_yolov8n_pose.py) 显示多人检测框、17 个关键点和骨架；将 `models/sdcard/yolov8n_pose_192_u55_256.tflite` 复制到板端存储根目录即可，无需额外解码脚本或标签文件。`App FPS` 统计包含采集、推理、后处理和 LCD 显示的完整循环。

本端口没有开放 RTC、WDT、DAC、CAN、I2S、SPI/I2C 从机、USB HID 或原始 USB_VCP API，因此不收录这些其他板专用示例。I2C1 读取 LSM6DS3 身份使用通用 I2C，不能视为已开放 imu 模块。

GPIO、SoftSPI、SoftI2C 示例之间复用针脚，不能直接拼接同时运行。软件定时器和 GPIO IRQ 在 VM 调度器处理，不是硬实时控制。硬件 ADC、PWM、SPI、UART 脚本在退出时调用 deinit；硬件 I2C 由板级保持，不关闭相机共享总线。

脚本许可与来源说明保留在各文件头，许可全文见 [LICENSE](../LICENSE)。
