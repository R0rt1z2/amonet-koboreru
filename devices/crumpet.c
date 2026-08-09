#include <device.h>
#include <patch.h>
#include <tee.h>

#include <drivers/kpd.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x00217F2C, 0);

    // Do not override USB descriptors with the originals.
    patch_word(0x00217548, 0xBF00447D);

    // Make sure the ARB check never runs (seriously,
    // ARB on this device USES FUSES).
    patch_ret(0x002019B0, 0);

    // This function is used to burn the new ARB version
    // to the devices fuses. Patch it to never run too,
    // just to be safe.
    patch_ret(0x00201954, 0);

    // Replace the TEE image loader with our own
    patch_branch(0x0020E19C, bldr_load_tee_part);
}

uint8_t usbdl_detect_key(void)
{
    // Force hacked USBDL for now.
    return 1;
}
