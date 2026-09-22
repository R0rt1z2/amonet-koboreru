#include <arb.h>
#include <bootmode.h>
#include <device.h>
#include <mmio.h>
#include <patch.h>
#include <tee.h>

#include <drivers/pwrap.h>

static void pmic_rmw(uint16_t addr, uint16_t value, uint16_t mask, uint16_t shift)
{
    uint16_t cur = pwrap_read(addr);
    pwrap_write(addr, (cur & ~(mask << shift)) | (value << shift));
}

void led_blink_amber(void)
{
    pmic_rmw(0x102, 0, 1, 7);

    pmic_rmw(0x576, 1, 3, 8);
    pmic_rmw(0x576, 7, 7, 0xc);

    pmic_rmw(0x570, 1, 1, 0xe);

    pmic_rmw(0x56e, 0x1f, 0x1f, 8);

    pmic_rmw(0x572, 0, 1, 0xa);
    pmic_rmw(0x572, 1, 1, 9);
}

void apply_patches(void)
{
    led_blink_amber();

    // Ensure we can load unsigned DAs
    patch_ret(0x000d64fc, 0); // da_auth_init
    patch_ret(0x000d64b8, 0); // sec_auth

    // Do not override USB descriptors with the originals.
    patch_word(0x000d9348, 0xBF004B0C);

    // Replace the ARB check with a routine that clears the counters.
    patch_branch(0x000c1934, clear_rpmb_arb);

    // Replace the TEE image loader with our own
    patch_branch(0x000cd570, bldr_load_tee_part);
}

uint8_t usbdl_detect_key(void)
{
    if (readl(G_BOOT_MODE_ADDR) != FACTORY_BOOT)
        return 0;

    writel(NORMAL_BOOT, G_BOOT_MODE_ADDR);
    return 1;
}
