/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_CAMERA_H
#define TITAN_CAMERA_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define TITAN_CAMERA_RESET_PIN (0x0b00)
#define TITAN_CAMERA_POWER_PIN (0x070a)
#define TITAN_CAMERA_I2C_BUS "i2c0"
#define TITAN_CAMERA_MAX_WIDTH (640U)
#define TITAN_CAMERA_MAX_HEIGHT (480U)

typedef struct {
    uint16_t x, y, width, height;
    bool rgb565;
} titan_camera_config_t;

/* IRQ-safe sink. A returned buffer is reserved exclusively for VIN; a
 * completed buffer is detached from all mailboxes before notification. */
typedef struct {
    void *context;
    uint8_t *(*take_buffer)(void *context);
    void (*complete_buffer)(void *context, uint8_t *data, uint32_t sequence);
} titan_camera_sink_t;

int titan_camera_clock_start(uint32_t frequency);
int titan_camera_sensor_reset(void);
int titan_camera_sensor_size(uint16_t width, uint16_t height);
int titan_camera_open(const titan_camera_config_t *config, const titan_camera_sink_t *sink, size_t capacity);
int titan_camera_start(void);
int titan_camera_close(void);
void titan_camera_quiesce_irq(void);
void titan_camera_wait(unsigned milliseconds);
uint32_t titan_camera_error(void);
#endif
