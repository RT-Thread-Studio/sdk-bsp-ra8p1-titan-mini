/*
 * I2C 示例 - 读取板载 LSM6DS3TR-C IMU 传感器
 *
 * 通过 RT-Thread I2C 设备框架读取 LSM6DS3TR-C 的 WHO_AM_I、
 * 加速度、角速度与温度寄存器，并在串口打印解析结果。
 *
 * 硬件连接: Titan Board Mini 板载 LSM6DS3TR-C IMU 挂在软件 I2C 总线 i2c1 上
 *           (SCL=PIN1292, SDA=PIN1291)
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <math.h>
#include "rt_example.h"

#define DBG_TAG     "i2c.imu"
#define DBG_LVL     DBG_LOG
#include <rtdbg.h>

/* ---- LSM6DS3TR-C 寄存器与参数 -------------------------------------------- */
#define I2C_BUS_NAME                "i2c1"          /* 板载软 I2C 总线 */
#define LSM6DS3TR_C_I2C_ADDR        0x6A            /* 7-bit 从机地址 */
#define LSM6DS3TR_C_WHO_AM_I        0x0F            /* 设备 ID 寄存器, 应读出 0x6A */

#define LSM6DS3TR_C_CTRL1_XL        0x10            /* 加速度 ODR/FS */
#define LSM6DS3TR_C_CTRL2_G         0x11            /* 陀螺仪 ODR/FS */
#define LSM6DS3TR_C_CTRL3_C         0x12            /* BDU / IF_INC 等控制 */
#define LSM6DS3TR_C_STATUS_REG      0x1E            /* 数据就绪状态 */
#define LSM6DS3TR_C_OUT_TEMP_L      0x20            /* 温度低字节 */
#define LSM6DS3TR_C_OUTX_L_G        0x22            /* 陀螺仪 X 低字节 */
#define LSM6DS3TR_C_OUTX_L_XL       0x28            /* 加速度 X 低字节 */

/* CTRL1_XL: ODR=12.5Hz(0x20) | FS=±2g(0x00) -> 0x20
 * CTRL2_G : ODR=12.5Hz(0x20) | FS=±2000dps(0x00) -> 0x20
 * CTRL3_C : BDU=1(bit6) + IF_INC=1(bit2) -> 0x44, 复位值为 0x01 软复位(bit0)
 */
#define CTRL1_XL_VAL                0x20
#define CTRL2_G_VAL                 0x20
#define CTRL3_C_VAL                 0x44

/* 满量程换算: ±2g 时 1 LSB ≈ 0.061 mg; ±2000dps 时 1 LSB ≈ 70 mdps */
#define ACCEL_LSB_TO_MG             (0.061f)
#define GYRO_LSB_TO_MDPS            (70.0f)
#define TEMP_LSB_TO_DEGC            (0.0625f)       /* LSB→℃, 偏移 25℃ */
#define TEMP_OFFSET_DEGC            (25.0f)

#define MATH_PI                     3.14159265358979323846f

/* 数据就绪位 (STATUS_REG) */
#define STATUS_XLDA                 (1 << 0)        /* 加速度数据就绪 */
#define STATUS_GDA                  (1 << 1)        /* 陀螺仪数据就绪 */
#define STATUS_TDA                  (1 << 2)        /* 温度数据就绪 */

/* ---- 低层 I2C 读写 ------------------------------------------------------- */
static struct rt_i2c_bus_device *i2c_bus = RT_NULL;

/* 寄存器写: 单字节寄存器地址 + 数据 */
static rt_err_t imu_reg_write(rt_uint8_t reg, rt_uint8_t val)
{
    struct rt_i2c_msg msg;
    rt_uint8_t buf[2];

    buf[0] = reg;
    buf[1] = val;

    msg.addr  = LSM6DS3TR_C_I2C_ADDR;
    msg.flags = RT_I2C_WR;
    msg.buf   = buf;
    msg.len   = 2;

    if (rt_i2c_transfer(i2c_bus, &msg, 1) != 1)
        return -RT_ERROR;
    return RT_EOK;
}

/* 寄存器读: 先写寄存器地址, 再读 N 字节 (LSM6DS3TR-C 地址自增, IF_INC=1) */
static rt_err_t imu_reg_read(rt_uint8_t reg, rt_uint8_t *buf, rt_uint16_t len)
{
    struct rt_i2c_msg msgs[2];

    msgs[0].addr  = LSM6DS3TR_C_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    msgs[1].addr  = LSM6DS3TR_C_I2C_ADDR;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = buf;
    msgs[1].len   = len;

    if (rt_i2c_transfer(i2c_bus, msgs, 2) != 2)
        return -RT_ERROR;
    return RT_EOK;
}

static rt_int16_t combine_le(rt_uint8_t lo, rt_uint8_t hi)
{
    return (rt_int16_t)(((rt_uint16_t)hi << 8) | lo);
}

/* ---- 初始化 ------------------------------------------------------------- */
static rt_err_t imu_init(void)
{
    rt_uint8_t whoami = 0;

    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(I2C_BUS_NAME);
    if (i2c_bus == RT_NULL)
    {
        LOG_E("cannot find i2c bus %s", I2C_BUS_NAME);
        return -RT_ERROR;
    }

    if (imu_reg_read(LSM6DS3TR_C_WHO_AM_I, &whoami, 1) != RT_EOK)
    {
        LOG_E("i2c read WHO_AM_I failed");
        return -RT_ERROR;
    }

    if (whoami != LSM6DS3TR_C_I2C_ADDR)
    {
        LOG_E("IMU not found, WHO_AM_I=0x%02X (expected 0x6A)", whoami);
        return -RT_ERROR;
    }
    LOG_I("LSM6DS3TR-C detected, WHO_AM_I=0x%02X", whoami);

    /* 软件复位 */
    imu_reg_write(LSM6DS3TR_C_CTRL3_C, CTRL3_C_VAL | 0x01);
    rt_thread_mdelay(20);

    /* BDU=1, IF_INC=1 (地址自增) */
    imu_reg_write(LSM6DS3TR_C_CTRL3_C, CTRL3_C_VAL);

    /* 加速度: ODR=12.5Hz, FS=±2g */
    imu_reg_write(LSM6DS3TR_C_CTRL1_XL, CTRL1_XL_VAL);
    /* 陀螺仪: ODR=12.5Hz, FS=±2000dps */
    imu_reg_write(LSM6DS3TR_C_CTRL2_G, CTRL2_G_VAL);

    rt_thread_mdelay(50);   /* 等待第一帧数据稳定 */
    LOG_I("IMU configured: ODR=12.5Hz, accel ±2g, gyro ±2000dps");
    return RT_EOK;
}

/* ---- 读取并打印一次 ------------------------------------------------------ */
static rt_err_t imu_read_and_print(void)
{
    rt_uint8_t st = 0;
    rt_uint8_t tmp[6];
    rt_int16_t raw_ax, raw_ay, raw_az;
    rt_int16_t raw_gx, raw_gy, raw_gz;
    rt_int16_t raw_t;
    float ax_mg, ay_mg, az_mg, a_norm;
    float gx_dps, gy_dps, gz_dps, g_norm;
    float temp_c;
    float pitch, roll;

    /* 读状态, 等待任一数据就绪 (最多 200ms) */
    int wait = 0;
    do {
        if (imu_reg_read(LSM6DS3TR_C_STATUS_REG, &st, 1) != RT_EOK)
        {
            LOG_E("read STATUS_REG failed");
            return -RT_ERROR;
        }
        if (st & (STATUS_XLDA | STATUS_GDA | STATUS_TDA))
            break;
        rt_thread_mdelay(10);
    } while (++wait < 20);

    if ((st & (STATUS_XLDA | STATUS_GDA | STATUS_TDA)) == 0)
    {
        LOG_W("data not ready, status=0x%02X", st);
        return -RT_ERROR;
    }

    /* 加速度 X/Y/Z (顺序: XL, XH, YL, YH, ZL, ZH) */
    if (st & STATUS_XLDA)
    {
        if (imu_reg_read(LSM6DS3TR_C_OUTX_L_XL, tmp, 6) != RT_EOK)
        {
            LOG_E("read accel failed");
            return -RT_ERROR;
        }
        raw_ax = combine_le(tmp[0], tmp[1]);
        raw_ay = combine_le(tmp[2], tmp[3]);
        raw_az = combine_le(tmp[4], tmp[5]);
        ax_mg = raw_ax * ACCEL_LSB_TO_MG;
        ay_mg = raw_ay * ACCEL_LSB_TO_MG;
        az_mg = raw_az * ACCEL_LSB_TO_MG;
    }
    else
    {
        ax_mg = ay_mg = az_mg = 0.0f;
    }
    a_norm = sqrtf(ax_mg * ax_mg + ay_mg * ay_mg + az_mg * az_mg);

    /* 陀螺仪 X/Y/Z */
    if (st & STATUS_GDA)
    {
        if (imu_reg_read(LSM6DS3TR_C_OUTX_L_G, tmp, 6) != RT_EOK)
        {
            LOG_E("read gyro failed");
            return -RT_ERROR;
        }
        raw_gx = combine_le(tmp[0], tmp[1]);
        raw_gy = combine_le(tmp[2], tmp[3]);
        raw_gz = combine_le(tmp[4], tmp[5]);
        gx_dps = raw_gx * GYRO_LSB_TO_MDPS / 1000.0f;
        gy_dps = raw_gy * GYRO_LSB_TO_MDPS / 1000.0f;
        gz_dps = raw_gz * GYRO_LSB_TO_MDPS / 1000.0f;
    }
    else
    {
        gx_dps = gy_dps = gz_dps = 0.0f;
    }
    g_norm = sqrtf(gx_dps * gx_dps + gy_dps * gy_dps + gz_dps * gz_dps);

    /* 温度 */
    if (st & STATUS_TDA)
    {
        if (imu_reg_read(LSM6DS3TR_C_OUT_TEMP_L, tmp, 2) != RT_EOK)
        {
            LOG_E("read temp failed");
            return -RT_ERROR;
        }
        raw_t = combine_le(tmp[0], tmp[1]);
        temp_c = raw_t * TEMP_LSB_TO_DEGC + TEMP_OFFSET_DEGC;
    }
    else
    {
        temp_c = 0.0f;
    }

    /* 静态姿态角估算 (仅靠重力, 假设 X 朝前, Y 朝左, Z 朝上) */
    pitch = atan2f(-ax_mg, sqrtf(ay_mg * ay_mg + az_mg * az_mg)) * 180.0f / MATH_PI;
    roll  = atan2f(ay_mg, az_mg) * 180.0f / MATH_PI;

    rt_kprintf("------ LSM6DS3TR-C IMU Sample ------\n");
    rt_kprintf("Accel  (mg) : X=%7.1f  Y=%7.1f  Z=%7.1f  | |a|=%7.1f\n",
               ax_mg, ay_mg, az_mg, a_norm);
    rt_kprintf("Gyro   (dps): X=%7.2f  Y=%7.2f  Z=%7.2f  | |w|=%7.2f\n",
               gx_dps, gy_dps, gz_dps, g_norm);
    rt_kprintf("Temp   (C)  : %7.2f\n", temp_c);
    rt_kprintf("Tilt   (deg): pitch=%7.2f  roll=%7.2f  (static estimate)\n",
               pitch, roll);
    rt_kprintf("------------------------------------\n");
    return RT_EOK;
}

/* ---- MSH 命令 ----------------------------------------------------------- */
void i2c_sample(void)
{
    if (imu_init() != RT_EOK)
    {
        rt_kprintf("i2c imu init failed, please check i2c1 bus and sensor.\n");
        return;
    }

    if (imu_read_and_print() != RT_EOK)
    {
        rt_kprintf("i2c imu read failed.\n");
        return;
    }
}
MSH_CMD_EXPORT(i2c_sample, read LSM6DS3TR-C IMU sensor via i2c1);
