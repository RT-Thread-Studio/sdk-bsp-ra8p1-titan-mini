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

#define DISPLAY_SCREEN_WIDTH              (800)
#define DISPLAY_SCREEN_HEIGHT             (480)

#define BACKLIGHT_PWM_PERIOD      1000
#define BACKLIGHT_PWM_MIN         1000
#define BACKLIGHT_PWM_MAX         500
#define BRIGHTNESS_CHECK_INTERVAL  10

extern struct rt_completion ceu_completion;

static rt_uint32_t backlight_pwm_current = 1000;
uint8_t display_layer1_buff_select = 0;

static bool key_last_state = true;
static rt_tick_t key_debounce_tick = 0;
static bool key_processed = false;  // 添加：防止按键重复触发
#define KEY_DEBOUNCE_TIME   20
#define KEY_COOLDOWN_TIME   500

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

    camera_capture_start ();

    rt_pin_mode(KEY_PIN, PIN_MODE_INPUT_PULLUP);
    key_last_state = rt_pin_read(KEY_PIN);
#if defined(VIN_CFG_USE_RUNTIME_BUFFER)
    rt_kprintf("The vin driver uses hardware mailboxes for the buffer.\n");
#else
    rt_kprintf("The vin driver uses isr for the buffer.\n");
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
#ifndef VIN_CFG_USE_RUNTIME_BUFFER
        rt_completion_wait(&ceu_completion, RT_WAITING_FOREVER);
#endif

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

        /* Draw camera image to display buffer */
        uint16_t * p_src  = (uint16_t *)camera_data_ready_buffer_pointer_get();
        uint16_t * p_dest = (uint16_t *)&fb_background[display_layer1_buff_select][0];
        int x_offset = DISPLAY_SCREEN_WIDTH - CAMERA_CAPTURE_IMAGE_WIDTH;

        for(int y = 0; y < CAMERA_CAPTURE_IMAGE_HEIGHT; y++)
        {
            for(int x = 0; x < CAMERA_CAPTURE_IMAGE_WIDTH; x++)
            {
                *(p_dest + (y * DISPLAY_SCREEN_WIDTH) + (x_offset + x)) = *(p_src + (y * CAMERA_CAPTURE_IMAGE_WIDTH) + x);
            }
        }

        rt_thread_mdelay (1);
    }
}
