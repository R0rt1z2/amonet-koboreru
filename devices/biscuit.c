#include <device.h>
#include <patch.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x0021C5E4, 0);

    // Do not override USB descriptors with the originals.
    patch_word(0x00218094, 0xBF004628);

    // Make sure the ARB check never runs.
    patch_ret(0x00201934, 0);
}
