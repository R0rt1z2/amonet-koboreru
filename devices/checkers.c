#include <device.h>
#include <patch.h>
#include <preloader.h>
#include <tee.h>

#include <drivers/pmic_keys.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x0021B5E8, 0);

    // Do not override USB descriptors with the originals.
    patch_word(0x00216E20, 0xBF00447D);

    // Make sure the ARB check never runs.
    patch_ret(0x0020192C, 0);

    // Replace the TEE image loader with our own
    patch_branch(0x002091F8, bldr_load_tee_part);
}

uint8_t usbdl_detect_key(void)
{
    return pmic_key_pressed(USBDL_PMIC_KEY);
}
