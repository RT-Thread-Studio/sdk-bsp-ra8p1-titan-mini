#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>

#define DBG_TAG     "main"
#define DBG_LVL     DBG_INFO
#include <rtdbg.h>

#define LED_PIN_R   BSP_IO_PORT_01_PIN_09
#define LED_PIN_B   BSP_IO_PORT_01_PIN_10
#define LED_PIN_G   BSP_IO_PORT_01_PIN_08

#define LED_ON  (0)
#define LED_OFF (1)

#define PDM_FS_HZ                   (16000U)     /* 输出采样率 16kHz */
#define PDM_CHANNELS                (1U)         /* 单声道 */
#define PCM_16BITS                  (16U)
#define PCM_20BITS                  (20U)
#define PDM_FRAME_SAMPLES           ((1<<4) * PDM_CHANNELS)  /* 16 samples per frame */
#define PDM_ALIGN_UP(n,a)           (((uint32_t)((n) + (a) - 1U) / (uint32_t)(a)) * (uint32_t)(a))

#define PDM_RECORD_DURATION_SEC     (1U)         /* 每次录音 1 秒 */

#define PDM_BUFFER_NUM_SAMPLES      PDM_ALIGN_UP((uint32_t)(PDM_RECORD_DURATION_SEC * PDM_FS_HZ * PDM_CHANNELS), PDM_FRAME_SAMPLES)

typedef struct
{
    uint32_t samples;
    uint32_t bytes;
} pdm_buf_size_t;

#define NUM_BUFFERS  (2)
static int32_t g_pdm_buffer[NUM_BUFFERS][PDM_BUFFER_NUM_SAMPLES] = {0};
static int16_t g_dac_buffer[NUM_BUFFERS][PDM_BUFFER_NUM_SAMPLES * 2] = {0};

static volatile bool g_stop_receive_data = false;
static volatile pdm_error_t g_pdm_error = PDM_ERROR_NONE;
static volatile uint8_t g_write_buffer_index = 0;
static volatile uint8_t g_read_buffer_index = 0;
static volatile bool g_buffer_ready[NUM_BUFFERS] = {false, false};
static uint8_t g_pcm_bits = PCM_16BITS;

rt_device_t sound_dev;

static pdm_buf_size_t pdm_calc_buffer_from_seconds(uint8_t seconds)
{
    uint32_t raw = seconds * PDM_FS_HZ * PDM_CHANNELS;
    raw = raw > UINT32_MAX ? UINT32_MAX : raw;
    uint32_t samples = PDM_ALIGN_UP(raw, PDM_FRAME_SAMPLES);
    uint32_t bytes = samples * sizeof(int32_t);
    pdm_buf_size_t out = {samples, bytes};
    return out;
}

void pdm_callback(pdm_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case PDM_EVENT_DATA:
        {
            g_stop_receive_data = true;
            g_buffer_ready[g_write_buffer_index] = true;
            break;
        }

        case PDM_EVENT_SOUND_DETECTION:
        {
            LOG_I("PDM sound detection event");
            break;
        }

        case PDM_EVENT_ERROR:
        {
            g_pdm_error = p_args->error;
            LOG_E("PDM error: 0x%x", p_args->error);
            break;
        }

        default:
            break;
    }
}

static void convert_pdm_to_dac(int32_t *pdm_data, uint32_t pdm_samples, int16_t *dac_data)
{
    for (uint32_t i = 0; i < pdm_samples; i++)
    {
        int16_t sample_16;
        if (g_pcm_bits == PCM_20BITS)
        {
            sample_16 = (int16_t)(pdm_data[i] >> 4);
        }
        else
        {
            int32_t temp = pdm_data[i] << 1;
            sample_16 = (int16_t)temp;
        }

        dac_data[i * 2] = sample_16;
        dac_data[i * 2 + 1] = sample_16;
    }
}

void hal_entry(void)
{
    fsp_err_t err;
    pdm_buf_size_t buf_size;

    rt_kprintf("\n");
    rt_kprintf("********************************************************************************\n");
    rt_kprintf("*   PDM Microphone -> Speaker Real-time Loopback (Double Buffer)               *\n");
    rt_kprintf("********************************************************************************\n");
    rt_kprintf("Sample Rate: %d Hz, Channels: %d\n", PDM_FS_HZ, PDM_CHANNELS);
    rt_kprintf("Buffer: %d samples x %d buffers\n", PDM_BUFFER_NUM_SAMPLES, NUM_BUFFERS);
    rt_kprintf("********************************************************************************\n\n");

    rt_pin_mode(LED_PIN_R, PIN_MODE_OUTPUT);
    rt_pin_mode(LED_PIN_G, PIN_MODE_OUTPUT);
    rt_pin_mode(LED_PIN_B, PIN_MODE_OUTPUT);
    rt_pin_write(LED_PIN_R, LED_OFF);
    rt_pin_write(LED_PIN_G, LED_OFF);
    rt_pin_write(LED_PIN_B, LED_OFF);

    g_pcm_bits = (g_pdm0_cfg.pcm_width <= PDM_PCM_WIDTH_20_BITS_3_18) ? PCM_20BITS : PCM_16BITS;
    rt_kprintf("PDM PCM bits: %d\n", g_pcm_bits);

    err = R_PDM_Open(&g_pdm0_ctrl, &g_pdm0_cfg);
    if (err != FSP_SUCCESS)
    {
        LOG_E("R_PDM_Open failed: %d", err);
        return;
    }

    R_BSP_SoftwareDelay(841 + 35000, BSP_DELAY_UNITS_MICROSECONDS);

    sound_dev = rt_device_find("sound0");
    if (sound_dev != RT_NULL)
    {
        if (rt_device_open(sound_dev, RT_DEVICE_OFLAG_WRONLY) == RT_EOK)
        {
            LOG_I("Speaker opened");
        }
        else
        {
            LOG_W("Failed to open speaker");
            sound_dev = RT_NULL;
        }
    }
    else
    {
        LOG_W("Speaker device not found");
    }

    rt_pin_write(LED_PIN_G, LED_ON);
    rt_pin_write(LED_PIN_B, LED_ON);

    buf_size = pdm_calc_buffer_from_seconds(PDM_RECORD_DURATION_SEC);
    uint32_t loop_count = 0;
    g_write_buffer_index = 0;
    g_read_buffer_index = 0;
    g_stop_receive_data = false;
    g_pdm_error = PDM_ERROR_NONE;

    err = R_PDM_Start(&g_pdm0_ctrl, g_pdm_buffer[0], buf_size.bytes, buf_size.samples / 4);
    if (err != FSP_SUCCESS)
    {
        LOG_E("R_PDM_Start failed: %d", err);
        goto cleanup;
    }

    while (1)
    {
        while (!g_stop_receive_data)
        {
            rt_thread_mdelay(1);
        }

        err = R_PDM_Stop(&g_pdm0_ctrl);
        if (err != FSP_SUCCESS)
        {
            LOG_E("R_PDM_Stop failed: %d", err);
        }

        convert_pdm_to_dac(g_pdm_buffer[g_write_buffer_index], buf_size.samples,
                          g_dac_buffer[g_write_buffer_index]);

        uint8_t ready_idx = g_write_buffer_index;
        g_write_buffer_index = (g_write_buffer_index + 1) % NUM_BUFFERS;
        g_stop_receive_data = false;
        err = R_PDM_Start(&g_pdm0_ctrl, g_pdm_buffer[g_write_buffer_index],
                         buf_size.bytes, buf_size.samples / 4);
        if (err != FSP_SUCCESS)
        {
            LOG_E("R_PDM_Start failed: %d", err);
            break;
        }
        if (sound_dev != RT_NULL)
        {
            rt_size_t total_bytes = buf_size.samples * 2;
            rt_size_t chunk_size = 512;
            rt_size_t offset = 0;

            while (offset < total_bytes)
            {
                rt_size_t remaining = total_bytes - offset;
                rt_size_t write_size = (remaining < chunk_size) ? remaining : chunk_size;

                rt_size_t bytes_written = rt_device_write(sound_dev, 0,
                                                             (uint8_t*)g_dac_buffer[ready_idx] + offset,
                                                             write_size);

                if (bytes_written > 0)
                {
                    offset += bytes_written;
                }
                else
                {
                    rt_thread_mdelay(1);
                }
            }
        }
        loop_count++;
        if (loop_count % 50 == 0)
        {
            rt_pin_write(LED_PIN_B, !rt_pin_read(LED_PIN_B));
        }
    }

cleanup:
    R_PDM_Close(&g_pdm0_ctrl);
    if (sound_dev != RT_NULL)
    {
        rt_device_close(sound_dev);
    }
}
