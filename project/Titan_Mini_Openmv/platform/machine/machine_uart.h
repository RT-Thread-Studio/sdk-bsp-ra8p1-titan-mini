/* SPDX-License-Identifier: MIT */
#ifndef TITAN_MACHINE_UART_H
#define TITAN_MACHINE_UART_H
#include <stdbool.h>
#include <stdint.h>
#include "py/obj.h"
extern const mp_obj_type_t machine_uart_type;
void ra8_machine_uart_deinit_all(void);
bool ra8_uart_pin_owned(uint16_t pin);
#endif
