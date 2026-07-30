#include <device.h>
#include <patch.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_word(0x0021C5E4, THUMB_RET_ZERO);

    // Skip LK authentication
    patch_word(0x00211E30, THUMB_RET_ZERO);

    // Make sure we load tee2 instead of tee1
    patch_word(0x0020AF28, 0x00014ECD);

    // Always zero out the RPMB block (no ARB)
    patch_word(0x0020ABF4, 0xBF0058E6);
}
