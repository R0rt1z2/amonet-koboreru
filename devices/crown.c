#include <device.h>
#include <patch.h>
#include <preloader.h>
#include <tee.h>

#include <drivers/kpd.h>
#include <drivers/pmic_keys.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x0021B258, 0);

    // Do not override USB descriptors with the originals.
    patch_word(0x00216A90, 0xBF00447D);

    // Make sure the ARB check never runs.
    patch_ret(0x0020192C, 0);

    // Replace the TEE image loader with our own
    patch_branch(0x002091F4, bldr_load_tee_part);
}

uint8_t usbdl_detect_key(void)
{
    kpd_init(KPD_ROWS, KPD_COLS, KPD_DEBOUNCE_MS);

    return pmic_key_pressed(USBDL_PMIC_KEY)
        && kpd_key_pressed(KPD_KEY(VOL_DOWN_ROW, VOL_DOWN_COL))
        && !kpd_key_pressed(KPD_KEY(VOL_UP_ROW, VOL_UP_COL));
}
