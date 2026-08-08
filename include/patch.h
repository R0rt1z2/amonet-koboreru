#pragma once

#include <stdint.h>

void patch_word(uint32_t addr, uint32_t value);
void patch_ret(uint32_t addr, uint32_t value);
void patch_branch(uint32_t addr, const void *target);
