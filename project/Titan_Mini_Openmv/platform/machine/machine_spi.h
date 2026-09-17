/* SPDX-License-Identifier: MIT */
#ifndef TITAN_MACHINE_SPI_H
#define TITAN_MACHINE_SPI_H
#include <stdbool.h>
#include <stdint.h>
#include "py/obj.h"
extern const mp_obj_type_t machine_spi_type;
void ra8_machine_spi_deinit_all(void);
bool ra8_spi_pin_owned(uint16_t pin);
#endif
