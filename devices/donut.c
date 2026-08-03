#include <device.h>
#include <patch.h>

#include <drivers/kpd.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x002180E0, 0);

    // Do not override USB descriptors with the originals.
    patch_word(0x002176FC, 0xBF00447D);

    // Make sure the ARB check never runs.
    patch_ret(0x0020192C, 0);
}

uint8_t usbdl_detect_key(void)
{
    // Force hacked USBDL for now.
    return 1;
}
