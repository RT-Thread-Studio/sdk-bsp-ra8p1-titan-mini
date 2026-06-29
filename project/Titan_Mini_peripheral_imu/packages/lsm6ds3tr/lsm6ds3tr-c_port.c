/* Includes ------------------------------------------------------------------*/
#include "lsm6ds3tr-c_reg.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <rtthread.h>
#include <rtdevice.h>

/* Private macro -------------------------------------------------------------*/
#define    LSM6DS3_I2C_BUS_NAME      "i2c1"
#define    LSM6DS3_SAMPLE_INTERVAL_MS (1000)   /* 循环采样周期 */
#define    MATH_PI                    3.14159265358979323846

/* Private variables ---------------------------------------------------------*/
static struct rt_i2c_bus_device *i2c_bus = RT_NULL;
static stmdev_ctx_t dev_ctx;
static rt_bool_t lsm6ds3_inited = RT_FALSE;

/* 循环采样线程控制 */
static struct rt_thread sample_thread;
static rt_uint8_t sample_thread_stack[1024];
static rt_bool_t sample_running = RT_FALSE;
static rt_uint32_t sample_period_ms = LSM6DS3_SAMPLE_INTERVAL_MS;

/* Private functions ---------------------------------------------------------*/
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void platform_delay(uint32_t ms);
static void platform_init(void);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  初始化 LSM6DS3TR-C 传感器
 * @retval RT_EOK 成功, -RT_ERROR 失败
 */
int lsm6ds3_init(void)
{
    uint8_t whoamI = 0, rst = 0;

    /* Initialize mems driver interface */
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg  = platform_read;
    dev_ctx.mdelay    = platform_delay;

    /* Init test platform */
    platform_init();
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("[imu] i2c bus %s not found!\n", LSM6DS3_I2C_BUS_NAME);
        return -RT_ERROR;
    }

    /* Check device ID */
    lsm6ds3tr_c_device_id_get(&dev_ctx, &whoamI);
    if (whoamI != LSM6DS3TR_C_ID)
    {
        rt_kprintf("[imu] device not found, id=0x%02X (expected 0x%02X)\n",
                   whoamI, LSM6DS3TR_C_ID);
        return -RT_ERROR;
    }
    rt_kprintf("[imu] device detected, id=0x%02X\n", whoamI);

    /* Restore default configuration */
    lsm6ds3tr_c_reset_set(&dev_ctx, PROPERTY_ENABLE);
    do {
        lsm6ds3tr_c_reset_get(&dev_ctx, &rst);
    } while (rst);

    /* Enable Block Data Update */
    lsm6ds3tr_c_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
    /* Set Output Data Rate: 12.5Hz */
    lsm6ds3tr_c_xl_data_rate_set(&dev_ctx, LSM6DS3TR_C_XL_ODR_12Hz5);
    lsm6ds3tr_c_gy_data_rate_set(&dev_ctx, LSM6DS3TR_C_GY_ODR_12Hz5);
    /* Set full scale: accel ±2g, gyro ±2000dps */
    lsm6ds3tr_c_xl_full_scale_set(&dev_ctx, LSM6DS3TR_C_2g);
    lsm6ds3tr_c_gy_full_scale_set(&dev_ctx, LSM6DS3TR_C_2000dps);
    /* Accelerometer - analog filter */
    lsm6ds3tr_c_xl_filter_analog_set(&dev_ctx, LSM6DS3TR_C_XL_ANA_BW_400Hz);
    /* Accelerometer - LPF1 + LPF2 path */
    lsm6ds3tr_c_xl_lp2_bandwidth_set(&dev_ctx,
                                     LSM6DS3TR_C_XL_LOW_NOISE_LP_ODR_DIV_100);
    /* Gyroscope - filtering chain */
    lsm6ds3tr_c_gy_band_pass_set(&dev_ctx,
                                 LSM6DS3TR_C_HP_260mHz_LP1_STRONG);

    lsm6ds3_inited = RT_TRUE;
    rt_kprintf("[imu] initialized: ODR=12.5Hz, FS=±2g/±2000dps\n");
    return RT_EOK;
}

/* 开机自动初始化: 无需手动输入 'imu init', 上电即完成传感器初始化 */
static int lsm6ds3_auto_init(void)
{
    int ret = lsm6ds3_init();
    if (ret == RT_EOK)
        rt_kprintf("[imu] auto-init OK, run 'imu' to read sample\n");
    else
        rt_kprintf("[imu] auto-init failed\n");
    return ret;
}
INIT_APP_EXPORT(lsm6ds3_auto_init);

/**
 * @brief  读取并解析一次传感器数据
 * @param  accel_mg      输出: 加速度 X/Y/Z (mg)
 * @param  gyro_mdps     输出: 角速度 X/Y/Z (mdps)
 * @param  temp_degC     输出: 温度 (℃)
 * @retval RT_EOK 成功, -RT_ERROR 失败
 */
int lsm6ds3_read(float accel_mg[3], float gyro_mdps[3], float *temp_degC)
{
    int16_t data_raw_accel[3];
    int16_t data_raw_gyro[3];
    int16_t data_raw_temp;
    lsm6ds3tr_c_reg_t reg;

    if (!lsm6ds3_inited)
        return -RT_ERROR;

    /* 等待数据就绪 (最多等 200ms) */
    int wait = 0;
    do {
        lsm6ds3tr_c_status_reg_get(&dev_ctx, &reg.status_reg);
        if (reg.status_reg.xlda || reg.status_reg.gda || reg.status_reg.tda)
            break;
        rt_thread_mdelay(10);
    } while (++wait < 20);

    if (reg.status_reg.xlda)
    {
        rt_memset(data_raw_accel, 0x00, sizeof(data_raw_accel));
        lsm6ds3tr_c_acceleration_raw_get(&dev_ctx, data_raw_accel);
        accel_mg[0] = lsm6ds3tr_c_from_fs2g_to_mg(data_raw_accel[0]);
        accel_mg[1] = lsm6ds3tr_c_from_fs2g_to_mg(data_raw_accel[1]);
        accel_mg[2] = lsm6ds3tr_c_from_fs2g_to_mg(data_raw_accel[2]);
    }

    if (reg.status_reg.gda)
    {
        rt_memset(data_raw_gyro, 0x00, sizeof(data_raw_gyro));
        lsm6ds3tr_c_angular_rate_raw_get(&dev_ctx, data_raw_gyro);
        gyro_mdps[0] = lsm6ds3tr_c_from_fs2000dps_to_mdps(data_raw_gyro[0]);
        gyro_mdps[1] = lsm6ds3tr_c_from_fs2000dps_to_mdps(data_raw_gyro[1]);
        gyro_mdps[2] = lsm6ds3tr_c_from_fs2000dps_to_mdps(data_raw_gyro[2]);
    }

    if (reg.status_reg.tda)
    {
        data_raw_temp = 0;
        lsm6ds3tr_c_temperature_raw_get(&dev_ctx, &data_raw_temp);
        *temp_degC = lsm6ds3tr_c_from_lsb_to_celsius(data_raw_temp);
    }

    return RT_EOK;
}

/**
 * @brief  打印一次解析后的传感器信息（含加速度合矢量、姿态角估算）
 */
void lsm6ds3_print_sample(void)
{
    float a[3], g[3], t = 0.0f;

    if (lsm6ds3_read(a, g, &t) != RT_EOK)
    {
        rt_kprintf("[imu] not initialized (auto-init failed), check i2c1\n");
        return;
    }

    /* 加速度合矢量 (mg) */
    float a_norm = sqrtf(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);

    /* 由重力方向估算俯仰角 / 横滚角 (单位: 度)
     * 假设静止时仅有重力作用, X 朝前, Y 朝左, Z 朝上 */
    float pitch = atan2f(-a[0],
                         sqrtf(a[1]*a[1] + a[2]*a[2])) * 180.0f / MATH_PI;
    float roll  = atan2f(a[1], a[2]) * 180.0f / MATH_PI;

    /* 角速度合矢量 (mdps → dps) */
    float g_norm_dps = sqrtf(g[0]*g[0] + g[1]*g[1] + g[2]*g[2]) / 1000.0f;

    rt_kprintf("------ LSM6DS3TR-C Sample ------\n");
    rt_kprintf("Accel  (mg) : X=%6.1f  Y=%6.1f  Z=%6.1f  | |a|=%6.1f\n",
               a[0], a[1], a[2], a_norm);
    rt_kprintf("Gyro   (dps): X=%6.2f  Y=%6.2f  Z=%6.2f  | |w|=%6.2f\n",
               g[0]/1000.0f, g[1]/1000.0f, g[2]/1000.0f, g_norm_dps);
    rt_kprintf("Temp   (C)  : %6.2f\n", t);
    rt_kprintf("Tilt   (deg): pitch=%6.2f  roll=%6.2f  (static estimate)\n",
               pitch, roll);
    rt_kprintf("--------------------------------\n");
}

/* ---- 循环采样线程 (imu start/stop) -----------------------------------------*/
static void lsm6ds3_sample_thread_entry(void *param)
{
    while (sample_running)
    {
        lsm6ds3_print_sample();
        rt_thread_mdelay(sample_period_ms);
    }
}

/* ---- MSH 命令 --------------------------------------------------------------*/
/**
 * imu              - 打印一次采样
 * imu start [ms]   - 开始周期采样（可选周期，默认 1000ms）
 * imu stop         - 停止周期采样
 */
static int imu_cmd(int argc, char **argv)
{
    if (argc < 2)
    {
        lsm6ds3_print_sample();
        return 0;
    }

    if (!rt_strcmp(argv[1], "start"))
    {
        rt_uint32_t period = LSM6DS3_SAMPLE_INTERVAL_MS;
        if (argc >= 3)
            period = (rt_uint32_t)atoi(argv[2]);
        if (period < 50) period = 50;

        if (!lsm6ds3_inited)
        {
            rt_kprintf("[imu] not initialized, check wiring / i2c1 bus\n");
            return -1;
        }
        if (sample_running)
        {
            rt_kprintf("[imu] already running, stop it first\n");
            return -1;
        }

        sample_period_ms = period;
        sample_running = RT_TRUE;
        rt_thread_init(&sample_thread,
                       "imu_samp",
                       lsm6ds3_sample_thread_entry,
                       RT_NULL,
                       sample_thread_stack,
                       sizeof(sample_thread_stack),
                       20, 5);
        rt_thread_startup(&sample_thread);
        rt_kprintf("[imu] sampling started, period=%ums\n", period);
    }
    else if (!rt_strcmp(argv[1], "stop"))
    {
        if (!sample_running)
        {
            rt_kprintf("[imu] not running\n");
            return 0;
        }
        sample_running = RT_FALSE;
        rt_thread_mdelay(sample_period_ms + 50);
        rt_thread_detach(&sample_thread);
        rt_kprintf("[imu] sampling stopped\n");
    }
    else
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("  imu              read & print one sample\n");
        rt_kprintf("  imu start [ms]   start periodic sampling (default %ums)\n",
                   LSM6DS3_SAMPLE_INTERVAL_MS);
        rt_kprintf("  imu stop         stop periodic sampling\n");
        rt_kprintf("  (sensor is auto-initialized on boot)\n");
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(imu_cmd, imu, LSM6DS3 IMU sample command);

/* ========================================================================== */
/* Platform dependent low-level operations                                    */
/* ========================================================================== */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len)
{
    struct rt_i2c_msg msg;
    rt_uint8_t buf[1 + len];

    buf[0] = reg;
    rt_memcpy(&buf[1], bufp, len);

    msg.addr  = LSM6DS3TR_C_ID;
    msg.flags = RT_I2C_WR;
    msg.buf   = buf;
    msg.len   = len + 1;

    if (rt_i2c_transfer(i2c_bus, &msg, 1) == 1)
        return RT_EOK;
    else
        return -RT_ERROR;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
    struct rt_i2c_msg msgs[2];

    msgs[0].addr  = LSM6DS3TR_C_ID;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    msgs[1].addr  = LSM6DS3TR_C_ID;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = bufp;
    msgs[1].len   = len;

    if (rt_i2c_transfer(i2c_bus, msgs, 2) == 2)
        return RT_EOK;
    else
        return -RT_ERROR;
}

static void platform_delay(uint32_t ms)
{
    rt_thread_mdelay(ms);
}

static void platform_init(void)
{
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(LSM6DS3_I2C_BUS_NAME);
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("[imu] Error: I2C bus %s not found!\n",
                   LSM6DS3_I2C_BUS_NAME);
    }
}
