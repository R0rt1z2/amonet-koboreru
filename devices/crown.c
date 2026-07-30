#include <device.h>
#include <patch.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x0021B258, 0);

    // Make sure the ARB check never runs.
    patch_ret(0x0020192C, 0);
}
