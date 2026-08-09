#include <debug.h>
#include <preloader.h>

#include "tee.h"

#define ERR_NO_MTEE_HEADER (-771)

static int tee_verify_image(uint32_t *addr, uint32_t size)
{
    const struct tee_image_hdr *hdr = (const struct tee_image_hdr *)*addr;

    // Now we must decide whether this is a stock TEE image (encrypted)
    // or a custom (plaintext) one. If we detect the "MTK TEE " header then
    // call the decryption routine as usual.
    if (hdr->magic[0] == TEE_IMAGE_MAGIC0 && hdr->magic[1] == TEE_IMAGE_MAGIC1) {
        printf("[pl-payload] Detected stock TEE image.\n");
        return mtee_verify_decrypt(addr, *(uint32_t *)TEE_SECMEM_SIZE_ADDR, (uint8_t*)MTEE_IMG_VFY_PUBK_ADDR);
    }

    // Otherwise, we assume this is a raw image and do nothing, part_load
    // already puts the raw image at the ATF load address.
    return ERR_NO_MTEE_HEADER;
}

int bldr_load_tee_part(char *name, void *bdev, uint32_t *addr, uint32_t offset, uint32_t *size)
{   
    uint32_t next_offset, tee_addr = 0;
    void *part = part_get(name);
    int ret;

    if (!part)
        return -1;

    // Load the ARM trusted firmware (BL31).
    ret = part_load(bdev, part, addr, offset, size);
    if (ret) {
        printf("[pl-payload] Failed to load ATF sub-partition\n");
        return ret;
    }

    ret = tee_verify_image(addr, *size);
    if (ret == ERR_NO_MTEE_HEADER)
        return 0;

    if (ret) {
        printf("[pl-payload] Failed to verify ATF sub-partition.\n");
        return ret;
    }

    next_offset = 0x200 + *size; /* sizeof(part_hdr_t) */

    // Try to load the TEE sub-partition now. Note that failures from this
    // are NOT fatal since we can run without BL32 (given we are not running
    // the stock BL31 that will try to load it).
    ret = part_load(bdev, part, &tee_addr, next_offset, size);
    if (ret) {
        printf("[pl-payload] Failed to load TEE sub-partition.\n");
        return 0;
    }

    ret = tee_verify_image(&tee_addr, *size);
    if (ret) {
        printf("[pl-payload] Failed to verify TEE sub-partition.\n");
        return ret;
    }

    // Set the BL32 entry point.
    tee_set_entry(tee_addr);
    return 0;
}
