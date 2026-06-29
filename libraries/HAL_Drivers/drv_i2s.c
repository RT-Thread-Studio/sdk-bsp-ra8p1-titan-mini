
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_i2s.h"
#include "hal_data.h"
#include <rthw.h>
#include <math.h>
#include <string.h>
#include "r_gpt.h"
#include "r_ssi.h"
#include "es8156.h"
#define DBG_TAG              "i2s"
#define DBG_LVL              DBG_INFO
#include <rtdbg.h>

#define TX_FIFO_SIZE         (2048)
#define I2S_SUPPORTED_SAMPLE_RATE    (16000U)
#define I2S_SUPPORTED_SAMPLE_BITS    (16U)
#define I2S_SUPPORTED_CHANNELS       (2U)
#define I2S_SUPPORTED_MONO_CHANNELS  (1U)
#define GPT_AUDIO_MCLK_PERIOD_COUNTS (49U)
#define GPT_AUDIO_MCLK_DUTY_COUNTS   (24U)

volatile i2s_event_t g_last_i2s_event = I2S_EVENT_IDLE;
static rt_uint8_t g_i2s_tx_dma_buffer[TX_FIFO_SIZE] BSP_ALIGN_VARIABLE(32) BSP_PLACE_IN_SECTION(".ram_nocache");
static rt_uint8_t g_i2s_mono_expand_buffer[TX_FIFO_SIZE] BSP_ALIGN_VARIABLE(32) BSP_PLACE_IN_SECTION(".ram_nocache");

struct sound_device
{
    struct rt_audio_device audio;
    struct rt_audio_configure audio_config;
    char *dev_name;

    rt_uint8_t *tx_buff;
    rt_mq_t tx_mq;
    rt_sem_t tx_sem;

    rt_uint8_t *rx_buff;
    rt_uint8_t volume;
};
static struct sound_device snd_dev = {0};

bool music_player_active = false;
bool music_player_pause = false;

static fsp_err_t sound_configure_audio_clock(void)
{
    fsp_err_t err;

    err = R_GPT_Stop(&g_timer2_ctrl);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = R_GPT_PeriodSet(&g_timer2_ctrl, GPT_AUDIO_MCLK_PERIOD_COUNTS);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = R_GPT_DutyCycleSet(&g_timer2_ctrl, GPT_AUDIO_MCLK_DUTY_COUNTS, GPT_IO_PIN_GTIOCA);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = R_GPT_Reset(&g_timer2_ctrl);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = R_GPT_Start(&g_timer2_ctrl);
    return err;
}

static rt_size_t sound_expand_mono_to_stereo_16bit(const rt_uint8_t *src, rt_size_t src_size, rt_uint8_t *dst, rt_size_t dst_size)
{
    rt_size_t sample_bytes = sizeof(rt_int16_t);
    rt_size_t mono_samples = src_size / sample_bytes;
    rt_size_t stereo_samples_capacity = dst_size / (sample_bytes * I2S_SUPPORTED_CHANNELS);
    rt_size_t samples_to_expand = mono_samples < stereo_samples_capacity ? mono_samples : stereo_samples_capacity;
    const rt_int16_t *src_samples = (const rt_int16_t *) src;
    rt_int16_t *dst_samples = (rt_int16_t *) dst;

    for (rt_size_t i = 0; i < samples_to_expand; i++)
    {
        rt_int16_t sample = src_samples[i];
        dst_samples[i * 2]     = sample;
        dst_samples[i * 2 + 1] = sample;
    }

    return samples_to_expand * sample_bytes * I2S_SUPPORTED_CHANNELS;
}


void i2s0_callback(i2s_callback_args_t *p_args)
{
    extern struct sound_device snd_dev;

    g_last_i2s_event = p_args->event;
    if (I2S_EVENT_TX_EMPTY == p_args->event)
    {
        rt_audio_tx_complete(&snd_dev.audio);
    }
}

static rt_err_t sound_getcaps(struct rt_audio_device *audio, struct rt_audio_caps *caps)
{
    rt_err_t result = RT_EOK;
    struct sound_device *snd_dev;

    RT_ASSERT(audio != RT_NULL);
    snd_dev = (struct sound_device *)audio->parent.user_data;

    switch (caps->main_type)
    {
    case AUDIO_TYPE_QUERY:
    {
        switch (caps->sub_type)
        {
        case AUDIO_TYPE_QUERY:
            break;

        default:
            result = -RT_ERROR;
            break;
        }
        break;
    }

    case AUDIO_TYPE_OUTPUT:
    {
        switch (caps->sub_type)
        {
        case AUDIO_DSP_PARAM:
            caps->udata.config.samplerate   = snd_dev->audio_config.samplerate;
            caps->udata.config.channels     = snd_dev->audio_config.channels;
            caps->udata.config.samplebits   = snd_dev->audio_config.samplebits;
            break;

        case AUDIO_DSP_SAMPLERATE:
            caps->udata.config.samplerate   = snd_dev->audio_config.samplerate;
            break;

        case AUDIO_DSP_CHANNELS:
            caps->udata.config.channels     = snd_dev->audio_config.channels;
            break;

        case AUDIO_DSP_SAMPLEBITS:
            caps->udata.config.samplebits   = snd_dev->audio_config.samplebits;
            break;

        default:
            result = -RT_ERROR;
            break;
        }
        break;
    }

    case AUDIO_TYPE_MIXER:
    {
        switch (caps->sub_type)
        {
        case AUDIO_MIXER_QUERY:
            caps->udata.mask = AUDIO_MIXER_VOLUME;
            break;

        case AUDIO_MIXER_VOLUME:
            snd_dev->volume = caps->udata.value;
            break;

        default:
            result = -RT_ERROR;
            break;
        }
        break;
    }

    default:
        result = -RT_ERROR;
        break;
    }

    return result;
}

static rt_err_t sound_configure(struct rt_audio_device *audio, struct rt_audio_caps *caps)
{
    rt_err_t result = RT_EOK;
    struct sound_device *snd_dev;

    RT_ASSERT(audio != RT_NULL);
    snd_dev = (struct sound_device *)audio->parent.user_data;

    switch (caps->main_type)
    {
    case AUDIO_TYPE_MIXER:
    {
        switch (caps->sub_type)
        {
        case AUDIO_MIXER_VOLUME:
        {
            rt_uint8_t volume = caps->udata.value;
            int vol_pct = volume;
            if (vol_pct < 0)   vol_pct = 0;
            if (vol_pct > 100) vol_pct = 100;
            snd_dev->volume = vol_pct;
            /* 0~100% → 0~255，191=0dB */
            es8156_set_volume((rt_uint8_t)(255 / 100 * vol_pct));
            rt_kprintf("set volume %d", volume);
            break;
        }

        default:
            result = -RT_ERROR;
            break;
        }

        break;
    }

    case AUDIO_TYPE_OUTPUT:
    {
        switch (caps->sub_type)
        {
        case AUDIO_DSP_PARAM:
        {
            snd_dev->audio_config.samplerate   = caps->udata.config.samplerate;
            snd_dev->audio_config.channels     = caps->udata.config.channels;
            snd_dev->audio_config.samplebits   = caps->udata.config.samplebits;
            rt_kprintf("AUDIO_DSP_PARAM:set samplerate %d", snd_dev->audio_config.samplerate);
            if ((snd_dev->audio_config.samplerate != I2S_SUPPORTED_SAMPLE_RATE) ||
                ((snd_dev->audio_config.channels != I2S_SUPPORTED_CHANNELS) &&
                 (snd_dev->audio_config.channels != I2S_SUPPORTED_MONO_CHANNELS)) ||
                (snd_dev->audio_config.samplebits != I2S_SUPPORTED_SAMPLE_BITS))
            {
                LOG_W("unsupported wav format: %d Hz, %d ch, %d bit; driver is fixed at %d Hz, %d ch, %d bit",
                      snd_dev->audio_config.samplerate,
                      snd_dev->audio_config.channels,
                      snd_dev->audio_config.samplebits,
                      I2S_SUPPORTED_SAMPLE_RATE,
                      I2S_SUPPORTED_CHANNELS,
                      I2S_SUPPORTED_SAMPLE_BITS);
            }
            break;
        }
        case AUDIO_DSP_SAMPLERATE:
        {
            snd_dev->audio_config.samplerate = caps->udata.config.samplerate;
            rt_kprintf("AUDIO_DSP_SAMPLERATE:set samplerate %d", snd_dev->audio_config.samplerate);
            break;
        }
        case AUDIO_DSP_CHANNELS:
        {
            break;
        }
        case AUDIO_DSP_SAMPLEBITS:
        {
            /* not support */
            break;
        }
        default:
            result = -RT_ERROR;
            break;
        }
        break;
    }

    default:
        break;
    }

    return result;
}

static rt_err_t sound_init(struct rt_audio_device *audio)
{
    rt_err_t result = RT_EOK;
    struct sound_device *snd_dev;

    R_GPT_Open(&g_timer2_ctrl, &g_timer2_cfg);
    R_GPT_Enable(&g_timer2_ctrl);

    fsp_err_t err = sound_configure_audio_clock();
    if (FSP_SUCCESS != err)
    {
        LOG_E("GPT audio clock configure failed: %d", err);
        return -RT_ERROR;
    }

    es8156_device_init();

    err = R_SSI_Open(&g_i2s0_ctrl, &g_i2s0_cfg);
    if (FSP_SUCCESS != err)
    {
        LOG_E("SSI Open failed: %d", err);
        return -RT_ERROR;
    }

    RT_ASSERT(audio != RT_NULL);
    snd_dev = (struct sound_device *)audio->parent.user_data;
    rt_kprintf("Audio init success.\r\n");
    return result;
}

static rt_err_t sound_start(struct rt_audio_device *audio, int stream)
{
    RT_ASSERT(audio != RT_NULL);
    struct sound_device *snd_dev;

    snd_dev = (struct sound_device *)audio->parent.user_data;
    if (stream == AUDIO_STREAM_REPLAY)

        rt_audio_tx_complete(audio); 
        
    
    return RT_EOK;
}

static rt_ssize_t sound_transmit(struct rt_audio_device *audio, const void *writeBuf, void *readBuf, rt_size_t size)
{
    struct sound_device *snd_dev;
    const rt_uint8_t *tx_data = (const rt_uint8_t *) writeBuf;
    rt_size_t tx_size = size;

    RT_ASSERT(audio != RT_NULL);
    snd_dev = (struct sound_device *)audio->parent.user_data;
    if (size > 0)
    {
        if ((snd_dev->audio_config.channels == I2S_SUPPORTED_MONO_CHANNELS) &&
            (snd_dev->audio_config.samplebits == I2S_SUPPORTED_SAMPLE_BITS))
        {
            tx_size = sound_expand_mono_to_stereo_16bit(tx_data, size, g_i2s_mono_expand_buffer, sizeof(g_i2s_mono_expand_buffer));
            tx_data = g_i2s_mono_expand_buffer;
        }

#if BSP_CFG_DCACHE_ENABLED
        /* Audio playback uses SSI + transfer engine, so flush CPU writes before DMA reads the buffer. */
        SCB_CleanDCache_by_Addr((uint32_t *) tx_data, (int32_t) tx_size);
#endif
        fsp_err_t err = R_SSI_Write(&g_i2s0_ctrl,
                                tx_data,
                                tx_size);
        if (FSP_SUCCESS != err)
        {
            return 0;
        }
    }
    return size;
}

static rt_err_t sound_stop(struct rt_audio_device *audio, int stream)
{
    RT_ASSERT(audio != RT_NULL);

    if (stream == AUDIO_STREAM_REPLAY)
    {
        R_SSI_Stop(&g_i2s0_ctrl);
        rt_kprintf("Sound Stop.\r\n");
    }

    return RT_EOK;
}

static void sound_buffer_info(struct rt_audio_device *audio, struct rt_audio_buf_info *info)
{
    struct sound_device *snd_dev;
    RT_ASSERT(audio != RT_NULL);
    snd_dev = (struct sound_device *)audio->parent.user_data;

    /**
     *               TX_FIFO
     * +----------------+----------------+
     * |     block1     |     block2     |
     * +----------------+----------------+
     *  \  block_size  /
     */
    info->buffer      = snd_dev->tx_buff;
    info->total_size  = TX_FIFO_SIZE;
    info->block_size  = TX_FIFO_SIZE/2;
    info->block_count = 2;
}
static struct rt_audio_ops snd_ops =
{
    .getcaps     = sound_getcaps,
    .configure   = sound_configure,
    .init        = sound_init,
    .start       = sound_start,
    .stop        = sound_stop,
    .transmit    = sound_transmit,
    .buffer_info = sound_buffer_info
};

int rt_hw_sound_init(void)
{
    rt_err_t ret = RT_EOK;

    rt_memset(g_i2s_tx_dma_buffer, 0, TX_FIFO_SIZE);
    snd_dev.tx_buff = g_i2s_tx_dma_buffer;
    snd_dev.audio_config.samplerate = 16000;
    snd_dev.audio_config.channels   = 2;
    snd_dev.audio_config.samplebits = 16;
    snd_dev.volume                   = 60;

    snd_dev.audio.ops = &snd_ops;

    ret = rt_audio_register(&snd_dev.audio, "sound0", RT_DEVICE_FLAG_WRONLY, &snd_dev);

    if (ret != RT_EOK)
    {
        LOG_E("rt_audio %s register failed, status=%d\r\n", "sound0", ret);
        return -RT_ERROR;
    }

    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_sound_init);
