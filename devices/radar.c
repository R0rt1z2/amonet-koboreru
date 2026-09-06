#include <arb.h>
#include <device.h>
#include <patch.h>
#include <tee.h>

#include <drivers/pmic_keys.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x0021C324, 0);

    // Do not override USB descriptors with the originals.
    patch_word(0x00217DD4, 0xBF004628);

    // Replace the ARB check with a routine that clears the counters.
    patch_branch(0x00201934, clear_rpmb_arb);

    // Replace the TEE image loader with our own
    patch_branch(0x0020A850, bldr_load_tee_part);
}

uint8_t usbdl_detect_key(void)
{
    return pmic_key_pressed(USBDL_PMIC_KEY);
}
