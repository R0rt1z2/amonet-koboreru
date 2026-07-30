#include <stdint.h>

#include <mmio.h>
#include <patch.h>

void patch_word(uint32_t addr, uint32_t value)
{
    writel(value, addr);
    invalidate_icache_range(addr, sizeof(value));
}
