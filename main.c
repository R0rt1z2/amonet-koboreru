#include <debug.h>
#include <device.h>
#include <mmio.h>

static void jump_back(void)
{
    // We need to jump back in ARM state ;(
    __asm__ volatile("ldr r4, [%0]\n\t"
                     "bx %1\n\t"
                     :
                     : "r"(PRELOADER_BOOT_ARG), "r"(PRELOADER_ENTRY)
                     : "r4");
    __builtin_unreachable();
}

int main(void *dev, uint32_t blk, uint32_t count, void *dst, uint32_t part)
{
    printf("\nThis is PL-payload by R0rt1z2 and bengris32. Copyright 2026\n");
    printf("Built at %s and running on %s @ 0x%08lX\n", __TIME__, DEVICE_NAME,
           (unsigned long)__builtin_return_address(0));

    // Show what we actually overwrote and who invoked us
    printf("\nread(dev %p, blk %lu, count %lu, dst %p, part %lu)\n", dev,
           (unsigned long)blk, (unsigned long)count, dst, (unsigned long)part);
    hexdump((const void *)BDEV_ADDR, 0x30);

    apply_patches();
    jump_back();

    while (1) {}
}
