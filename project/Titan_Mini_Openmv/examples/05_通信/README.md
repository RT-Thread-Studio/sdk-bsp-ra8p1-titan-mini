# UART 视觉协议

| 分类 | 支持范围 |
|---|---|
| 01_Arduino | UART2 文本发送与外部 Arduino 回显 |
| 02_Pixy | Pixy v1 普通色块、AprilTag color-code block 单向输出 |
| 03_MAVLink | MAVLink v1 OPTICAL_FLOW、LANDING_TARGET 基本负载输出 |

UART 例均使用 H1 的 UART2：H1/3=P801 TX、H1/2=P802 RX、H1/1=GND，3.3 V TTL；排针可能需自行焊接。USB CDC 保留给 IDE，UART1 保留给 msh。Arduino 等 5 V 器件发往板子的信号必须先转换电平。

Pixy 例保留官方目标数据帧格式和校验，只实现单向输出，不实现 Servo/DAC、下行 LED/舵机命令、颜色组合、多设备从机接口。普通色块的 LAB 阈值应按实际场景调整；AprilTag 例使用 TAG36H11，signature 为 ID+8。

MAVLink 报文字段依据 [MAVLink common.xml](https://mavlink.io/en/messages/common.html)。光流单位为 0.1 像素，无测距时距离为负值；本例没有陀螺补偿或真实速度估计。AprilTag 目标例需要先填写当前镜头和输出尺寸的内参、标签实际边长；未标定会报明确的参数错误。它们用于串口接收与算法台架验证，不包含完整 MAVLink 心跳、配置、命令或飞控集成。

当前不收录 Modbus 示例：官方脚本依赖未冻结的 modbus 库，其串口接收与帧完整性还需为本端口单独验证。RPC 官方库依赖本构建未启用的 Viper/native emit，I2C/SPI 从机也未实现。

当前固件没有向 Python 导出 `protocol` 模块，因此不提供官方 12-Protocol 用户通道例；固件内部的 USB/IDE 通讯不受影响。
