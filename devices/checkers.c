#include <device.h>
#include <patch.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x0021B5E8, 0);

    // Do not override USB descriptors with the originals.
    patch_word(0x00216E20, 0xBF00447D);

    // Make sure the ARB check never runs.
    patch_ret(0x0020192C, 0);
}
