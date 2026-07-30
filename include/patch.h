#pragma once

#include <stdint.h>

#define THUMB_RET_ZERO 0x47702000

void patch_word(uint32_t addr, uint32_t value);
