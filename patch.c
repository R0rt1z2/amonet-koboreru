#include <stdint.h>

#include <mmio.h>
#include <patch.h>

#define THUMB_MOVS_R0 0x2000
#define THUMB_LDR_R0  0x4800
#define THUMB_BX_LR   0x4770

#define THUMB_B_W     0x9000
#define THUMB_BL      0xD000

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

static void patch_thumb_branch(uint32_t addr, const void *target, uint16_t op)
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

    /* The site may only be halfword aligned, so no 32-bit store. */
    writew((uint16_t)(0xF000 | (s << 10) | imm10), addr);
    writew((uint16_t)(op | (j1 << 13) | (j2 << 11) | imm11), addr + 2);
    invalidate_icache_range(addr, 4);
}

void patch_branch(uint32_t addr, const void *target)
{
    patch_thumb_branch(addr, target, THUMB_B_W);
}

void patch_bl(uint32_t addr, const void *target)
{
    patch_thumb_branch(addr, target, THUMB_BL);
}
