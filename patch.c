#include <stddef.h>
#include <stdint.h>

#include <mmio.h>
#include <patch.h>

void patch_word(uint32_t addr, uint32_t value)
{
    writel(value, addr);
    invalidate_icache_range(addr, sizeof(value));
}

void apply_patches(const struct patch *patches, size_t count)
{
    for (size_t i = 0; i < count; i++)
        patch_word(patches[i].addr, patches[i].value);
}
