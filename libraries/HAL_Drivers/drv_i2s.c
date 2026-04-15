
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

#define TX_FIFO_SIZE         (1024)

/* 双缓冲区，用于中断驱动的音频传输 */
static rt_uint8_t g_stream_src[TX_FIFO_SIZE];
uint32_t g_buffer_index = 0;
static volatile bool g_data_ready = false;
static volatile bool g_send_data_in_main_loop = false;


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

void i2s0_callback(i2s_callback_args_t *p_args)
{
    extern struct sound_device snd_dev;
    if (I2S_EVENT_TX_EMPTY == p_args->event ||I2S_EVENT_IDLE == p_args->event){
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
            /* save configs */
            snd_dev->audio_config.samplerate = caps->udata.config.samplerate;
            snd_dev->audio_config.channels   = caps->udata.config.channels;
            snd_dev->audio_config.samplebits = caps->udata.config.samplebits;
            // sound_set_samplerate(snd_dev->audio_config.samplerate, snd_dev->audio_config.channels);
            rt_kprintf("AUDIO_DSP_PARAM:set samplerate %d", snd_dev->audio_config.samplerate);
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
    R_GPT_Start(&g_timer2_ctrl);

    es8156_device_init();

    fsp_err_t err = FSP_SUCCESS;
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
    RT_ASSERT(audio != RT_NULL);
    snd_dev = (struct sound_device *)audio->parent.user_data;
    if (size > 0)
    {

        if(snd_dev->audio_config.channels == 1)
        {
            rt_uint32_t *src = (rt_uint32_t *)writeBuf;
            rt_uint32_t *dst = (rt_uint32_t *)g_stream_src;
            for (int i = 0; i < size/4; i++)
            {
                dst[i * 4] = 0;
                dst[i * 4+4] = 0;
            }

            fsp_err_t err = R_SSI_Write(&g_i2s0_ctrl,
                        (uint8_t *) g_stream_src,
                        size*4);
            if (FSP_SUCCESS != err)
            {
                rt_kprintf("SSI Write also failed: %d", err);
            }    
        }
        else
        {
            fsp_err_t err = R_SSI_Write(&g_i2s0_ctrl,
                                    (uint8_t *) writeBuf,
                                    size);
            if (FSP_SUCCESS != err)
            {
                rt_kprintf("SSI Write also failed: %d", err);
            }    

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
    rt_uint8_t *tx_buff;

    tx_buff = (rt_uint8_t *)rt_malloc(TX_FIFO_SIZE);

    rt_memset(tx_buff, 0, TX_FIFO_SIZE);
    if (tx_buff == RT_NULL)
        return -RT_ENOMEM;
    snd_dev.tx_buff = tx_buff;
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
