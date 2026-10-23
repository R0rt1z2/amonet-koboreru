#include <arb.h>
#include <atf.h>
#include <device.h>
#include <patch.h>
#include <preloader.h>
#include <tee.h>

#include <drivers/kpd.h>

void apply_patches(void)
{
    // Ensure we can load unsigned DAs
    patch_ret(0x000d9080, 0); // sec_auth
    patch_ret(0x000d90c4, 0); // da_auth_init

    // Do not override USB descriptors with the originals
    patch_word(0x000dc764, 0xBF00447A);

    // Replace the ARB check with a routine that clears the counters
    patch_branch(0x000c1934, clear_rpmb_arb);

    // Replace the TEE image loader with our own
    patch_branch(0x000cee38, bldr_load_tee_part);

    // Hand a mainline BL31 the boot information it expects.
    patch_branch(0x000cf224, bldr_jump64);

    // Don't let part_load re-init DRAM and collapse the 2GB decode.
    patch_branch(0x000CC192, (const void *)0x000CC2C8);
}

uint8_t usbdl_detect_key(void)
{
    kpd_init(KPD_ROWS, KPD_COLS, KPD_DEBOUNCE_MS);

    // Force USB download when both volume keys are held together.
    return kpd_key_pressed(KPD_KEY(VOL_UP_ROW, VOL_UP_COL))
        && kpd_key_pressed(KPD_KEY(VOL_DOWN_ROW, VOL_DOWN_COL));
}
