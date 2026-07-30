#include <device.h>
#include <patch.h>

static const char swdl[] = "swdl";

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x0021B258, 0);

    // Skip LK authentication
    patch_ret(0x0020FD50, 0);

    // Make sure we load tee2 instead of tee1
    patch_word(0x00209794, 0x00014EB7);

    // Always zero out the RPMB block (no ARB)
    patch_word(0x002094C8, 0xBF0058E6);

    // Load LK from swdl on the next run
    patch_word(0x0020978C, (uint32_t)swdl - (0x00209586 + 4));
}
