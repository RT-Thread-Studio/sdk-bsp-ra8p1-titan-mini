/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2024-03-11     kurisaW       first version
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>
#include <board.h>
#include "camera_layer.h"
#include "camera_layer_config.h"

#define KEY_PIN     BSP_IO_PORT_02_PIN_01
#define DISPLAY_USE_DIRECT_VIN_BUFFER   (0)

#if DISPLAY_USE_DIRECT_VIN_BUFFER
#define DISPLAY_CAMERA_X_OFFSET   (160)
#define DISPLAY_CAMERA_Y_OFFSET   (0)
#else
#define DISPLAY_SCREEN_WIDTH              (800)
#define DISPLAY_SCREEN_HEIGHT             (480)
#define CAMERA_FRAME_LINE_BYTES           (CAMERA_CAPTURE_IMAGE_WIDTH * sizeof(uint16_t))
#endif

#define BACKLIGHT_PWM_PERIOD      1000
#define BACKLIGHT_PWM_MIN         1000
#define BACKLIGHT_PWM_MAX         500
#define BRIGHTNESS_CHECK_INTERVAL  10

extern struct rt_completion ceu_completion;
static struct rt_completion display_vsync_completion;

static rt_uint32_t backlight_pwm_current = 1000;
#if !DISPLAY_USE_DIRECT_VIN_BUFFER
static uint8_t display_layer1_buff_select = 0;
#endif

static bool key_last_state = true;
static rt_tick_t key_debounce_tick = 0;
static bool key_processed = false;  // 添加：防止按键重复触发
#define KEY_DEBOUNCE_TIME   20
#define KEY_COOLDOWN_TIME   500

void DisplayVsyncCallback(display_callback_args_t * p_args)
{
    if ((NULL != p_args) && (DISPLAY_EVENT_LINE_DETECTION == p_args->event))
    {
        rt_completion_done(&display_vsync_completion);
    }
}

void hal_entry(void)
{
    rt_kprintf ("\nHello Titan Board!\n");
    rt_kprintf ("===========================================================\n");
    rt_kprintf ("This example project is an mipi-csi camera display routine!\n");
    rt_kprintf ("===========================================================\n");

    struct rt_device_pwm *pwm12_dev;
    pwm12_dev = (struct rt_device_pwm *) rt_device_find("pwm12");
    if (pwm12_dev == RT_NULL)
    {
        return;
    }
    rt_pwm_enable(pwm12_dev, 0);
    rt_pwm_set(pwm12_dev, 0, BACKLIGHT_PWM_PERIOD, backlight_pwm_current);

    // Initialize camera interface
    fsp_err_t fsp_status = FSP_SUCCESS;
    fsp_status = camera_init(false);
    if(FSP_SUCCESS != fsp_status)
    {
        rt_kprintf ("camera_init fail!\n");
        return;
    }

    camera_image_buffer_initialize ();

    fsp_status = R_VIN_CaptureStart(&g_cam_vin_ctrl, NULL);
    if(FSP_SUCCESS != fsp_status)
    {
        rt_kprintf ("R_VIN_CaptureStart fail! err=%d\n", fsp_status);
        return;
    }

    /* Static VIN mailboxes need one completed frame before software copy is valid. */
    rt_thread_mdelay(100);

    rt_pin_mode(KEY_PIN, PIN_MODE_INPUT_PULLUP);
    key_last_state = rt_pin_read(KEY_PIN);
    rt_kprintf("The vin driver uses hardware mailboxes for the buffer.\n");
    rt_completion_init(&display_vsync_completion);

#if DISPLAY_USE_DIRECT_VIN_BUFFER
    g_display0_runtime_cfg_bg.input.p_base  = (uint32_t *)&vin_image_buffer_1[0];
    g_display0_runtime_cfg_bg.input.hsize   = CAMERA_CAPTURE_IMAGE_WIDTH;
    g_display0_runtime_cfg_bg.input.vsize   = CAMERA_CAPTURE_IMAGE_HEIGHT;
    g_display0_runtime_cfg_bg.input.hstride = VIN_CFG_IMAGE_STRIDE;
    g_display0_runtime_cfg_bg.layer.coordinate.x = DISPLAY_CAMERA_X_OFFSET;
    g_display0_runtime_cfg_bg.layer.coordinate.y = DISPLAY_CAMERA_Y_OFFSET;
    fsp_status = R_GLCDC_LayerChange(&g_display0_ctrl, &g_display0_runtime_cfg_bg, DISPLAY_FRAME_LAYER_1);
    if (FSP_SUCCESS != fsp_status)
    {
        rt_kprintf("R_GLCDC_LayerChange fail! err=%d\n", fsp_status);
        return;
    }
#else
    rt_memset(&fb_background[0][0], 0x00, sizeof(fb_background));
#endif

    static rt_tick_t focus_done_tick = 0;
    static rt_tick_t last_trigger_tick = 0;
    static bool focus_locked = false;
    const rt_tick_t release_delay = rt_tick_from_millisecond(600);

    rt_kprintf("[AF] Press key to trigger auto focus!\n");
    rt_pwm_set(pwm12_dev, 0, BACKLIGHT_PWM_PERIOD, 880);
    rt_thread_mdelay (1000);
    rt_pwm_set(pwm12_dev, 0, BACKLIGHT_PWM_PERIOD, 1000);
    while (1)
    {

        rt_tick_t current_tick = rt_tick_get();

        bool key_current = rt_pin_read(KEY_PIN);
        if (key_current != key_last_state) {
            key_debounce_tick = current_tick;
            key_processed = false;
        }
        key_last_state = key_current;

        if (!key_current && !key_processed &&
            (current_tick - key_debounce_tick) >= rt_tick_from_millisecond(KEY_DEBOUNCE_TIME)) {

            key_processed = true;

            if ((current_tick - last_trigger_tick) >= rt_tick_from_millisecond(KEY_COOLDOWN_TIME)) {

                rt_kprintf("[AF] Triggering auto focus...\n");
                int af_result = OV5640_auto_focus();
                if (af_result == RT_EOK) {
                    focus_locked = true;
                    focus_done_tick = current_tick;
                    last_trigger_tick = current_tick;
                    rt_kprintf("[AF] Focus locked!\n");
                }
            }
        }

        if (focus_locked && (current_tick - focus_done_tick) >= release_delay){
            uint8_t val;
            rdSensorReg16_8(0x3029, &val);
            if (val == 0x10)
                wrSensorReg16_8( 0x3022, 0x06);
            focus_locked = false;
            rt_kprintf("[I/AF] Release Focus\n");
        }

        if (!camera_capture_post_process())
        {
            rt_thread_mdelay (1);
            continue;
        }

#if DISPLAY_USE_DIRECT_VIN_BUFFER
        uint8_t * p_frame = (uint8_t *) camera_data_ready_buffer_pointer_get();
        if (FSP_SUCCESS == R_GLCDC_BufferChange(&g_display0_ctrl,
                                                p_frame,
                                                DISPLAY_FRAME_LAYER_1))
        {
            rt_completion_wait(&display_vsync_completion, rt_tick_from_millisecond(20));
        }
#else
        uint8_t next_buffer = (uint8_t)(display_layer1_buff_select ^ 1U);
        uint16_t * p_src  = (uint16_t *)camera_data_ready_buffer_pointer_get();
        uint16_t * p_dest = (uint16_t *)&fb_background[next_buffer][0];
        int x_offset = DISPLAY_SCREEN_WIDTH - CAMERA_CAPTURE_IMAGE_WIDTH;

        for (int y = 0; y < CAMERA_CAPTURE_IMAGE_HEIGHT; y++)
        {
            uint16_t * p_dest_row = p_dest + (y * DISPLAY_SCREEN_WIDTH) + x_offset;
            uint16_t * p_src_row  = p_src + (y * CAMERA_CAPTURE_IMAGE_WIDTH);

            rt_memcpy(p_dest_row, p_src_row, CAMERA_FRAME_LINE_BYTES);
        }

        if (FSP_SUCCESS == R_GLCDC_BufferChange(&g_display0_ctrl,
                                                (uint8_t *)&fb_background[next_buffer][0],
                                                DISPLAY_FRAME_LAYER_1))
        {
            if (RT_EOK == rt_completion_wait(&display_vsync_completion, rt_tick_from_millisecond(20)))
            {
                display_layer1_buff_select = next_buffer;
            }
        }
#endif
    }
}
