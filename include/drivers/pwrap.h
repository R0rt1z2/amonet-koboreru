#pragma once

#include <stdint.h>

#ifndef PWRAP_BASE
#error "PWRAP_BASE must be defined"
#endif

uint16_t pwrap_read(uint16_t addr);
void pwrap_write(uint16_t addr, uint16_t data);
