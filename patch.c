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
