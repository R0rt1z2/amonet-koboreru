#include <device.h>
#include <patch.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_word(0x0021B5E8, THUMB_RET_ZERO);

    // Skip LK authentication
    patch_word(0x0020FD50, THUMB_RET_ZERO);

    // Make sure we load tee2 instead of tee1
    patch_word(0x00209794, 0x00015243);

    // Always zero out the RPMB block (no ARB)
    patch_word(0x002094C8, 0xBF0058E6);
}
