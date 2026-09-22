/* SPDX-License-Identifier: Apache-2.0 */
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/gc.h"
#include "umalloc.h"
#include "led.h"
#include "board_config.h"
#include "framebuffer.h"
#include "ra8_capture.h"
#include "titan_storage.h"

typedef struct { mp_obj_base_t base; int index; bool value; } board_led_t;
static mp_obj_t led_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    int index = mp_obj_get_int(args[0]) - 1;
    if (index < 0 || index >= OMV_BOARD_LED_COUNT) { mp_raise_ValueError(MP_ERROR_TEXT("LED id must be 1, 2 or 3")); }
    board_led_t *self = mp_obj_malloc(board_led_t, type); self->index = index; self->value = false;
    led_state(index, false); return MP_OBJ_FROM_PTR(self);
}
static mp_obj_t led_on(mp_obj_t obj) { board_led_t *self = MP_OBJ_TO_PTR(obj); led_state(self->index, self->value = true); return mp_const_none; }
static mp_obj_t led_off(mp_obj_t obj) { board_led_t *self = MP_OBJ_TO_PTR(obj); led_state(self->index, self->value = false); return mp_const_none; }
static mp_obj_t led_toggle(mp_obj_t obj) { board_led_t *self = MP_OBJ_TO_PTR(obj); led_state(self->index, self->value = !self->value); return mp_const_none; }
static MP_DEFINE_CONST_FUN_OBJ_1(led_on_obj, led_on);
static MP_DEFINE_CONST_FUN_OBJ_1(led_off_obj, led_off);
static MP_DEFINE_CONST_FUN_OBJ_1(led_toggle_obj, led_toggle);
static const mp_rom_map_elem_t led_dict[] = {
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&led_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&led_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_toggle), MP_ROM_PTR(&led_toggle_obj) },
};
static MP_DEFINE_CONST_DICT(led_locals, led_dict);
MP_DEFINE_CONST_OBJ_TYPE(board_led_type, MP_QSTR_LED, MP_TYPE_FLAG_NONE,
    make_new, led_make_new, locals_dict, &led_locals);
static mp_obj_t board_info(void) {
    mp_obj_t tuple[] = {mp_obj_new_int_from_uint(SystemCoreClock),
        mp_obj_new_bool(SCB->CCR & SCB_CCR_IC_Msk), mp_obj_new_bool(SCB->CCR & SCB_CCR_DC_Msk),
        mp_obj_new_int(__ARM_FEATURE_MVE), mp_obj_new_str(MICROPY_HW_BOARD_NAME, sizeof(MICROPY_HW_BOARD_NAME) - 1)};
    return mp_obj_new_tuple(5, tuple);
}
static MP_DEFINE_CONST_FUN_OBJ_0(board_info_obj, board_info);
static uint32_t sdram_clock_hz(void) {
    if (!R_SYSTEM->SDCKOCR_b.SDCKOEN) { return 0; }
    if (!R_SYSTEM->BCKCR_b.EBCKASEL) {
        return R_FSP_SystemClockHzGet(FSP_PRIV_CLOCK_BCLK);
    }
    /* RA8P1 HW manual 9.2.60/9.2.64: decode the live selector/divider.
     * This reports nominal Hz from the BSP source clock table, not a
     * frequency measurement. BCKCR.BCLKDIV divides EBCLK, not SDCLK. */
    static const uint8_t dividers[] = {1, 2, 4, 6, 8, 0, 0, 10, 16, 32};
    unsigned divider = R_SYSTEM->BCKADIVCR_b.CKDIV;
    unsigned source = R_SYSTEM->BCKACR_b.CKSEL;
    if (divider >= sizeof(dividers) || !dividers[divider] ||
        (source != 1 && (source < 5 || source > 10)) ||
        R_SYSTEM->BCKACR_b.CKSREQ || R_SYSTEM->BCKACR_b.CKSRDY) {
        return 0;
    }
    return R_BSP_SourceClockHzGet((fsp_priv_source_clock_t)source) / dividers[divider];
}
static mp_obj_t board_memory(void) {
    gc_info_t gc;
    gc_info(&gc);
    size_t uma_free_bytes = 0;
    for (int i = 0; i < uma_pool_count(); ++i) {
        uma_stats_t stats;
        uma_get_stats(i, false, &stats);
        uma_free_bytes += stats.free_bytes;
    }
    mp_obj_t result = mp_obj_new_dict(10);
    mp_obj_dict_store(result, MP_OBJ_NEW_QSTR(MP_QSTR_sdram_clock_hz),
                      mp_obj_new_int_from_uint(sdram_clock_hz()));
    mp_obj_dict_store(result, MP_OBJ_NEW_QSTR(MP_QSTR_sdram_clock_async),
                      mp_obj_new_bool(R_SYSTEM->BCKCR_b.EBCKASEL));
    mp_obj_dict_store(result, MP_OBJ_NEW_QSTR(MP_QSTR_dcache_force_wt),
                      mp_obj_new_bool(MEMSYSCTL->MSCR & MEMSYSCTL_MSCR_FORCEWT_Msk));
    mp_obj_dict_store(result, MP_OBJ_NEW_QSTR(MP_QSTR_gc_total), mp_obj_new_int_from_uint(gc.total));
    mp_obj_dict_store(result, MP_OBJ_NEW_QSTR(MP_QSTR_gc_free), mp_obj_new_int_from_uint(gc.free));
    mp_obj_dict_store(result, MP_OBJ_NEW_QSTR(MP_QSTR_gc_largest_free),
                      mp_obj_new_int_from_uint(gc.max_free * MICROPY_BYTES_PER_GC_BLOCK));
    mp_obj_dict_store(result, MP_OBJ_NEW_QSTR(MP_QSTR_uma_free), mp_obj_new_int_from_uint(uma_free_bytes));
    #ifdef RT_USING_MEMHEAP_AS_HEAP
    rt_size_t size = 0, used = 0, peak = 0;
    rt_memory_info(&size, &used, &peak);
    rt_size_t free_bytes = size >= used ? size - used : 0;
    mp_obj_dict_store(result, MP_OBJ_NEW_QSTR(MP_QSTR_rt_system_heap_size), mp_obj_new_int_from_uint(size));
    mp_obj_dict_store(result, MP_OBJ_NEW_QSTR(MP_QSTR_rt_system_heap_free), mp_obj_new_int_from_uint(free_bytes));
    mp_obj_dict_store(result, MP_OBJ_NEW_QSTR(MP_QSTR_rt_system_heap_peak), mp_obj_new_int_from_uint(peak));
    #endif
    return result;
}
static MP_DEFINE_CONST_FUN_OBJ_0(board_memory_obj, board_memory);
static mp_obj_t board_preview(size_t n_args, const mp_obj_t *args) {
    framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);
    bool previous = fb->enabled;
    if (n_args) { framebuffer_set_enabled(fb, mp_obj_is_true(args[0])); }
    return mp_obj_new_bool(previous);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(board_preview_obj, 0, 1, board_preview);
static mp_obj_t board_pipeline(size_t n_args, const mp_obj_t *args) {
    return mp_obj_new_bool(ra8_capture_pipeline(n_args ? mp_obj_is_true(args[0]) : -1));
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(board_pipeline_obj, 0, 1, board_pipeline);
/* Optional medium load/eject control; selected storage is exported when ready.
 * Local FAT stays mounted. Avoid simultaneous host/script filesystem writes. */
static mp_obj_t board_usb_msc(size_t n_args, const mp_obj_t *args) {
    if (n_args) {
        int error = mp_obj_is_true(args[0]) ? titan_storage_usb_export() : titan_storage_usb_release();
        if (error < 0) { mp_raise_OSError(-error); }
    }
    return mp_obj_new_bool(titan_storage_usb_enabled());
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(board_usb_msc_obj, 0, 1, board_usb_msc);
static const mp_rom_map_elem_t board_dict[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_board) },
    { MP_ROM_QSTR(MP_QSTR_LED), MP_ROM_PTR(&board_led_type) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&board_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_memory), MP_ROM_PTR(&board_memory_obj) },
    { MP_ROM_QSTR(MP_QSTR_preview), MP_ROM_PTR(&board_preview_obj) },
    { MP_ROM_QSTR(MP_QSTR_pipeline), MP_ROM_PTR(&board_pipeline_obj) },
    { MP_ROM_QSTR(MP_QSTR_usb_msc), MP_ROM_PTR(&board_usb_msc_obj) },
};
static MP_DEFINE_CONST_DICT(board_globals, board_dict);
const mp_obj_module_t board_module = {{&mp_type_module}, (mp_obj_dict_t *)&board_globals};
MP_REGISTER_MODULE(MP_QSTR_board, board_module);
