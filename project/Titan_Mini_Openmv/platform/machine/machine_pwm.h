/* SPDX-License-Identifier: MIT */
#ifndef TITAN_MACHINE_PWM_H
#define TITAN_MACHINE_PWM_H
#include <stdbool.h>
#include <stdint.h>
#include "py/obj.h"
extern const mp_obj_type_t machine_pwm_type;
void machine_pwm_deinit_all(void);
bool ra8_pwm_pin_owned(uint16_t pin);
#endif
