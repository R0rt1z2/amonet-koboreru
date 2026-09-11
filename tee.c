#include <debug.h>
#include <mmio.h>
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
        return mtee_verify_decrypt(addr, *(uint32_t *)TEE_SECMEM_SIZE_ADDR, (uint8_t*)MTEE_IMG_VFY_PUBK_ADDR);
    }

    // Otherwise, we assume this is a raw image and do nothing, part_load
    // already puts the raw image at the ATF load address.
    return ERR_NO_MTEE_HEADER;
}

int bldr_load_tee_part(char *name, void *bdev, uint32_t *addr, uint32_t offset, uint32_t *size)
{   
    uint32_t next_offset, tee_addr;
    uint8_t bl31_raw;
    void *part = part_get(name);
    int ret;

    if (!part)
        return -1;

    // Load the ARM trusted firmware (BL31).
    ret = part_load(bdev, part, addr, offset, size);
    if (ret)
        return ret;

    ret = tee_verify_image(addr, *size);
    if (ret && ret != ERR_NO_MTEE_HEADER)
        return ret;

    bl31_raw = (ret == ERR_NO_MTEE_HEADER);
    next_offset = 0x200 + *size; /* sizeof(part_hdr_t) */

    // We can know if this is a stock TZ image by checking if
    // BL31 was encrypted or not. If it isn't then we can
    // safely assume that BL32 isn't either, and so load it
    // at a chosen fixed address.
    tee_addr = bl31_raw ? TEE_LOAD_ADDR : 0;

    ret = part_load(bdev, part, &tee_addr, next_offset, size);
    if (ret)
        return 0;

    ret = tee_verify_image(&tee_addr, *size);
    if (ret == ERR_NO_MTEE_HEADER) {
        // We need to set tee_secmem_size ourselves in this
        // case.
        writel(TEE_MEM_SIZE, TEE_SECMEM_SIZE_ADDR);
    } else if (ret) {
        return ret;
    } else if (bl31_raw) {
        // If BL31 lacked an MTEE header but BL32 didn't, then
        // something seriously weird is going on.
        return -1;
    }

    tee_set_entry(tee_addr);
    return 0;
}
