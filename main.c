#include <debug.h>
#include <device.h>
#include <mmio.h>
#include <preloader.h>
#include <usbdl.h>

// int (*bread)(blkdev_t *bdev, u32 blknr, u32 blks, u8 *buf, u32 part_id);
int main(void *dev, uint32_t blk, uint32_t count, void *dst, uint32_t part)
{
    int ret;
    uint32_t addr = LK_LOAD_ADDR, size = 0;

    printf("\nLook where you're jumping next time...\n");
    printf("\n");
    printf("           .--._.--.            \n");
    printf("          ( O     O )           \n");
    printf("          /   . .   \\          \n");
    printf("         .`._______.'.          \n");
    printf("        /(           )\\        \n");
    printf("      _/  \\  \\   /  /  \\_    \n");
    printf("   .~   `  \\  \\ /  /  '   ~.  \n");
    printf("  {    -.   \\  V  /   .-    }  \n");
    printf("_ _`.    \\  |  |  |  /    .'_ _\n");
    printf(">_       _} |  |  | {_       _< \n");
    printf(" /. - ~ ,_-'  .^.  `-_, ~ - .\\ \n");
    printf("         '-'|/   \\|`-`         \n\n");
    printf("-------------------------------------------------------------\n");
    printf("This is PL-payload by R0rt1z2 and bengris32. Copyright 2026\n");
    printf("Built at %s and running on %s @ 0x%08lX\n", __TIME__, DEVICE_NAME,
           (unsigned long)__builtin_return_address(0));
    printf("-------------------------------------------------------------\n");
    printf("\n");

#ifdef DEBUG
    // Show what we actually overwrote and who invoked us
    printf("\nread(dev %p, blk %lu, count %lu, dst %p, part %lu)\n", dev,
           (unsigned long)blk, (unsigned long)count, dst, (unsigned long)part);
    hexdump((const void *)BDEV_ADDR, 0x30);
#endif

    apply_patches();

    // We NOP the str instruction that actually sets up the pointer
    // to the original descriptor table, so we setup our own regardless
    // of whether or not we will enter usbdl here, in case we end up in
    // there outside of enter_usbdl().
    setup_usb_descriptors();

    // Enter unsecured USB Download Mode if requested.
    enter_usbdl();

    // Restore the g_dram_buf pointer stored in SRAM since basically
    // everything depends on it from where we are.
    writel(DRAM_BUF, SRAM_DBUF_PTR);

    // We need to restore the bdev ops before handing back to Preloader.
    // We can do that by first erasing the list head of the bdev list
    // and then calling boot_device_init(), which will eventually reset
    // g_mmc_bdev and call blkdev_register(&g_mmc_bdev), restoring the
    // contents of the bdev list to before we overwrote it.
    writel(0, BDEV_LH_ADDR);
    boot_device_init();

    // Preloader loads LK first before TEE, and if LK fails to load the
    // it immediately resets, therefore a valid / signed LK image has to
    // be present in the original LK partition.
    //
    // As we are therefore running AFTER LK has been loaded, we need to
    // re-load the LK from whatever partition is used on the device for
    // the "real" one.
    ret = bldr_load_part(LK_PART_NAME, dev, &addr, &size);
    if (ret)
        printf("*** Failed to load new LK: %d ***\n", ret);

    // Then hand back with a negative value to indicate a read error.
    // This will cause Preloader to fall back to the TEE2 partition,
    // which is what we want!
    return -1;
}
