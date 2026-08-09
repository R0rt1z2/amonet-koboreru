#include <device.h>
#include <patch.h>
#include <preloader.h>
#include <tee.h>

#include <drivers/kpd.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x0021B5F8, 0);

    // Do not override USB descriptors with the originals.
    patch_word(0x00216E30, 0xBF00447D);

    // Make sure the ARB check never runs.
    patch_ret(0x0020192C, 0);

    // Replace the TEE image loader with our own
    patch_branch(0x002092C8, bldr_load_tee_part);
}

uint8_t usbdl_detect_key(void)
{
    kpd_init(KPD_ROWS, KPD_COLS, KPD_DEBOUNCE_MS);
    return kpd_key_pressed(KPD_KEY(USBDL_KEY_ROW, USBDL_KEY_COL));
}
