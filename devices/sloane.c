#include <arb.h>
#include <atf.h>
#include <bootmode.h>
#include <device.h>
#include <mmio.h>
#include <patch.h>
#include <preloader.h>
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

uint32_t part_load_addr(const struct part_hdr *hdr, uint32_t maddr,
                        const uint32_t *addr)
{
    if (maddr == PART_HEADER_DEFAULT_ADDR)
        return *addr;

    if (hdr->mode == LOAD_ADDR_MODE_BACKWARD)
        return tee_get_load_addr(maddr);

    return maddr;
}

__attribute__((naked))
static void part_load_hook(void)
{
    __asm__ volatile(
        "push {r3, lr}\n"
        "mov  r0, r4\n"
        "mov  r1, r10\n"
        "mov  r2, r6\n"
        "bl   part_load_addr\n"
        "mov  r4, r0\n"
        "pop  {r3, pc}\n");
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

    // Hand a mainline BL31 the boot information it expects
    patch_branch(0x000cd69c, bldr_jump64);

    // Let a partition header pick its own load address again
    patch_bl(0x000d28ae, part_load_hook);
}

uint8_t usbdl_detect_key(void)
{
    if (readl(G_BOOT_MODE_ADDR) != FACTORY_BOOT)
        return 0;

    writel(NORMAL_BOOT, G_BOOT_MODE_ADDR);
    return 1;
}
