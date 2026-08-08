#include <stdint.h>

#include <mmio.h>
#include <patch.h>

#define THUMB_MOVS_R0 0x2000
#define THUMB_LDR_R0  0x4800
#define THUMB_BX_LR   0x4770

void patch_word(uint32_t addr, uint32_t value)
{
    writel(value, addr);
    invalidate_icache_range(addr, sizeof(value));
}

void patch_ret(uint32_t addr, uint32_t value)
{
    if (value <= 0xFF) {
        patch_word(addr, (THUMB_BX_LR << 16) | THUMB_MOVS_R0 | value);
        return;
    }

    patch_word(addr, (THUMB_BX_LR << 16) | THUMB_LDR_R0);
    patch_word(addr + 4, value);
}

void patch_branch(uint32_t addr, const void *target)
{
    int32_t off = (int32_t)(((uint32_t)target & ~1u) - (addr + 4));
    uint32_t u = (uint32_t)off;

    uint32_t s     = (u >> 24) & 1;
    uint32_t i1    = (u >> 23) & 1;
    uint32_t i2    = (u >> 22) & 1;
    uint32_t j1    = (~(i1 ^ s)) & 1;
    uint32_t j2    = (~(i2 ^ s)) & 1;
    uint32_t imm10 = (u >> 12) & 0x3FF;
    uint32_t imm11 = (u >> 1) & 0x7FF;

    uint16_t hi = (uint16_t)(0xF000 | (s << 10) | imm10);
    uint16_t lo = (uint16_t)(0x9000 | (j1 << 13) | (j2 << 11) | imm11);

    patch_word(addr, ((uint32_t)lo << 16) | hi);
}
