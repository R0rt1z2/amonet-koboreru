#pragma once

#include <stddef.h>
#include <stdint.h>

#define THUMB_RET_ZERO 0x47702000

struct patch {
    uint32_t addr;
    uint32_t value;
};

void patch_word(uint32_t addr, uint32_t value);
void apply_patches(const struct patch *patches, size_t count);
