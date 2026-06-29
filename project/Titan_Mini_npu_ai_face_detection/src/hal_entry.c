#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>

#include <board.h>
#include "camera_layer.h"
#include "camera_layer_config.h"
#include <lcd_port.h>
#include "model.h"
#include "pmu_ethosu.h"
#include "yolo_rtthread.h"

#define DBG_TAG     "main"
#define DBG_LVL     DBG_INFO
#include "rtdbg.h"

#define LED_PIN     BSP_IO_PORT_01_PIN_09
#define KEY_PIN     BSP_IO_PORT_02_PIN_01

#define CAM_WIDTH   640
#define CAM_HEIGHT  480
#define DETECT_RESULT_LOG_ENABLE        (0)
#define KEY_DEBOUNCE_TIME   20
#define KEY_COOLDOWN_TIME   500

static volatile bool led_status = false;
static struct rt_completion display_vsync_completion;
static bool key_last_state = true;
static rt_tick_t key_debounce_tick = 0;
static bool key_processed = false;

static void lcd_draw_jpg_with_frame(int32_t x, int32_t y,
                             const void *p, int32_t xSize, int32_t ySize,
                             uint32_t argb, d2_width thickness, det_box_t *boxes, int32_t box_num)
{
    if (thickness < 1)
    {
        thickness = 1;
    }

    uint32_t ModeSrc = d2_mode_rgb565;
    d2_device *hdl = d2_handle_obj_get();

    d2_framebuffer(hdl, (uint16_t *)&fb_background[0],
                   LCD_WIDTH, LCD_WIDTH, LCD_HEIGHT, ModeSrc);

    d2_selectrenderbuffer(hdl, d2_renderbuffer_get());
    d2_cliprect(hdl, 0, 0, LCD_WIDTH, LCD_HEIGHT);

    d2_setblitsrc(hdl, (void *)p, xSize, xSize, ySize, ModeSrc);
    d2_blitcopy(hdl, xSize, ySize, 0, 0,
            D2_FIX(LCD_WIDTH), D2_FIX(LCD_HEIGHT),
                D2_FIX(x), D2_FIX(y), 0);

    d2_setcolor(hdl, 0, argb);
    d2_width draw_thickness = D2_FIX(thickness);
    d2_u32 flags = d2_le_exclude_none;

    for (int i = 0; i < box_num; i++, boxes++)
    {
        d2_point x1 = D2_FIX(boxes->x1);
        d2_point y1 = D2_FIX(boxes->y1);
        d2_point x2 = D2_FIX(boxes->x2);
        d2_point y2 = D2_FIX(boxes->y2);

        d2_renderline(hdl, x1, y1, x2, y1, draw_thickness, flags);
        d2_renderline(hdl, x1, y1, x1, y2, draw_thickness, flags);
        d2_renderline(hdl, x2, y1, x2, y2, draw_thickness, flags);
        d2_renderline(hdl, x1, y2, x2, y2, draw_thickness, flags);
    }

    d2_executerenderbuffer(hdl, d2_renderbuffer_get(), 0);
    d2_flushframe(hdl);
}

void DisplayVsyncCallback(display_callback_args_t * p_args)
{
    if ((NULL != p_args) && (DISPLAY_EVENT_LINE_DETECTION == p_args->event))
    {
        rt_completion_done(&display_vsync_completion);
    }
}

void hal_entry(void)
{
    LOG_I("========================================================================");
    LOG_I("This example project is an mipi npu accelerated ai face detection routine!");
    LOG_I("========================================================================");

    fsp_err_t fsp_status = FSP_SUCCESS;

    fsp_status = camera_init(false);
    if (FSP_SUCCESS != fsp_status)
    {
        LOG_E("camera_init fail! err=%d", fsp_status);
        return;
    }

    camera_image_buffer_initialize();

    fsp_status = R_VIN_CaptureStart(&g_cam_vin_ctrl, NULL);
    if (FSP_SUCCESS != fsp_status)
    {
        LOG_E("R_VIN_CaptureStart fail! err=%d", fsp_status);
        return;
    }

    rt_thread_mdelay(100);
    rt_completion_init(&display_vsync_completion);
    rt_pin_mode(KEY_PIN, PIN_MODE_INPUT_PULLUP);
    key_last_state = rt_pin_read(KEY_PIN);

    int16_t status = FSP_SUCCESS;
    status = RM_ETHOSU_Open(&g_rm_ethosu0_ctrl, &g_rm_ethosu0_cfg);
    if (status != FSP_SUCCESS)
    {
        LOG_E("Failed to start NPU");
        return;
    }

    int8_t *in_i8 = (int8_t *)rt_malloc(INPUT_W * INPUT_H * sizeof(int8_t));
    if (!in_i8)
    {
        LOG_E("malloc in_i8 failed!");
        return;
    }

    float *out_f1 = (float *)rt_malloc(output1_len * sizeof(float));
    float *out_f2 = (float *)rt_malloc(output2_len * sizeof(float));
    if ((!out_f1) || (!out_f2))
    {
        LOG_E("malloc output buffer failed!");
        if (out_f1)
        {
            rt_free(out_f1);
        }
        if (out_f2)
        {
            rt_free(out_f2);
        }
        rt_free(in_i8);
        return;
    }

    static rt_tick_t focus_done_tick = 0;
    static rt_tick_t last_trigger_tick = 0;
    static bool focus_locked = false;
    const rt_tick_t release_delay = rt_tick_from_millisecond(600);

    rt_kprintf("[AF] Press key to trigger auto focus!\n");

    while (1)
    {
        rt_tick_t start = rt_tick_get();
        rt_tick_t current_tick = rt_tick_get();

        bool key_current = rt_pin_read(KEY_PIN);
        if (key_current != key_last_state)
        {
            key_debounce_tick = current_tick;
            key_processed = false;
        }
        key_last_state = key_current;

        if (!key_current && !key_processed &&
            (current_tick - key_debounce_tick) >= rt_tick_from_millisecond(KEY_DEBOUNCE_TIME))
        {
            key_processed = true;

            if ((current_tick - last_trigger_tick) >= rt_tick_from_millisecond(KEY_COOLDOWN_TIME))
            {
                rt_kprintf("[AF] Triggering auto focus...\n");
                if (OV5640_auto_focus() == RT_EOK)
                {
                    focus_locked = true;
                    focus_done_tick = current_tick;
                    last_trigger_tick = current_tick;
                    rt_kprintf("[AF] Focus locked!\n");
                }
            }
        }

        if (focus_locked && (current_tick - focus_done_tick) >= release_delay)
        {
            uint8_t val;
            rdSensorReg16_8(0x3029, &val);
            if (val == 0x10)
            {
                wrSensorReg16_8(0x3022, 0x06);
            }
            focus_locked = false;
            rt_kprintf("[I/AF] Release Focus\n");
        }

        if (!camera_capture_post_process())
        {
            rt_thread_mdelay(1);
            continue;
        }

        uint16_t *frame = (uint16_t *)camera_data_ready_buffer_pointer_get();
        if (!frame)
        {
            rt_thread_mdelay(1);
            continue;
        }

        rgb565_to_gray_resize_192_and_quantization(frame, CAM_WIDTH, CAM_HEIGHT, in_i8);

        memcpy(GetModelInputPtr_serving_default_image_input_0(), in_i8, INPUT_SIZE);

        RunModel(false);

        int8_t *output1 = GetModelOutputPtr_StatefulPartitionedCall_0_70273();
        int8_t *output2 = GetModelOutputPtr_StatefulPartitionedCall_1_70283();

        dequantize_int8(output1, out_f1, output1_len, scale_out1, zero_point_out1);
        dequantize_int8(output2, out_f2, output2_len, scale_out2, zero_point_out2);

        int16_t total = 0;
        static det_box_t pool[540];

        total += decode_output_layer(out_f1, GRID_SIZE_1, 0, LCD_WIDTH, LCD_HEIGHT, CONF_THRESH,
                pool + total, (int16_t)(sizeof(pool) / sizeof(pool[0])) - total);

        total += decode_output_layer(out_f2, GRID_SIZE_2, 1, LCD_WIDTH, LCD_HEIGHT, CONF_THRESH,
                pool + total, (int16_t)(sizeof(pool) / sizeof(pool[0])) - total);

        int32_t kept = nms_filter(pool, total, NMS_THRESH);
        int32_t out_n = MIN(kept, MAX_BOXES);

        d2_width thickness = 1;
        uint32_t argb = 0xFF00FF00;
        lcd_draw_jpg_with_frame(0, 0, frame, CAM_WIDTH, CAM_HEIGHT, argb, thickness, pool, out_n);

#if DETECT_RESULT_LOG_ENABLE
        rt_tick_t end = rt_tick_get();
        rt_kprintf("detect box num: %d\n", out_n);
        rt_kprintf("Time elapsed: %d ms\n", (end - start) * (1000 / RT_TICK_PER_SECOND));
#endif

        rt_thread_mdelay(5);

        led_status = !led_status;
        rt_pin_write(LED_PIN, led_status ? PIN_HIGH : PIN_LOW);
    }
}
