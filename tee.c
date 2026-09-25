#include <debug.h>
#include <mmio.h>
#include <preloader.h>

#include <libc/memory.h>

#include "atf.h"
#include "tee.h"

#define ERR_NO_MTEE_HEADER (-771)

#define PART_HDR_SIZE 0x200 /* sizeof(part_hdr_t) */

#ifdef TEE_SEC_CHUNK_OFF
/* The GCPU only reaches DRAM, but ATF loads to SRAM here, so stage it in
 * sec_chunk_buf and copy it over after. mt8163 verifies in place. */
#define TEE_STAGE_ATF 1

static inline uint32_t tee_stage_addr(void)
{
    return readl(SRAM_DBUF_PTR) + TEE_SEC_CHUNK_OFF;
}

#if defined(ATF_LOAD_ADDR) && defined(ATF_MAX_SIZE)
#define PAYLOAD_STACK_SIZE 0x1000
typedef char payload_clear_of_atf_window[
    ((PAYLOAD_ADDR - PAYLOAD_STACK_SIZE) >= (ATF_LOAD_ADDR + ATF_MAX_SIZE) ||
     BSS_START <= ATF_LOAD_ADDR) ? 1 : -1];
#endif
#endif

static int tee_verify_image(uint32_t *addr, uint32_t size)
{
    const struct tee_image_hdr *hdr = (const struct tee_image_hdr *)*addr;

    // Now we must decide whether this is a stock TEE image (encrypted)
    // or a custom (plaintext) one. If we detect the "MTK TEE " header then
    // call the decryption routine as usual.
    if (hdr->magic[0] == TEE_IMAGE_MAGIC0 && hdr->magic[1] == TEE_IMAGE_MAGIC1) {
        return mtee_verify_decrypt(addr, readl(TEE_SECMEM_SIZE_ADDR),
                                   (uint8_t *)MTEE_IMG_VFY_PUBK_ADDR);
    }

    // Otherwise, we assume this is a raw image and do nothing, part_load
    // already puts the raw image at the ATF load address.
    return ERR_NO_MTEE_HEADER;
}

static void tee_set_secmem(uint32_t start, uint32_t size)
{
    writel(size, TEE_SECMEM_SIZE_ADDR);

#ifdef TEE_SECMEM_START_ADDR
    writel(start, TEE_SECMEM_START_ADDR);
#endif
#ifdef TEE_EXTRA_MEM_SIZE_ADDR
    writel(0, TEE_EXTRA_MEM_SIZE_ADDR);
#endif
#ifdef ATF_LOG_BUF_START_ADDR
    writel(start, ATF_LOG_BUF_START_ADDR);
#endif
}

int bldr_load_tee_part(char *name, void *bdev, uint32_t *addr, uint32_t offset, uint32_t *size)
{
    uint32_t next_offset, tee_addr, atf_addr;
    uint32_t atf_skip = 0;
    uint8_t bl31_raw;
    void *part = part_get(name);
    int ret;
#ifdef TEE_STAGE_ATF
    uint32_t stage_addr = tee_stage_addr();
#endif

    if (!part)
        return -1;

#ifdef SEC_HEAP_USED_ADDR
    // The Preloader's secure heap is a bump allocator with no free, and it is
    // only reset by trustzone_pre_init(). Every image verified before us eats
    // into it, so reclaim it or mtee_verify_decrypt() runs out of room.
    writel(0, SEC_HEAP_USED_ADDR);
#endif

#ifdef TEE_STAGE_ATF
    atf_addr = stage_addr;
#else
    atf_addr = *addr;
#endif

    // Load the ARM trusted firmware (BL31).
    ret = part_load(bdev, part, &atf_addr, offset, size);
    if (ret)
        return ret;

    ret = tee_verify_image(&atf_addr, *size);
    if (ret && ret != ERR_NO_MTEE_HEADER)
        return ret;

    bl31_raw = (ret == ERR_NO_MTEE_HEADER);

#ifdef ATF_HANDOFF_ADDR
    if (atf_note_image(atf_addr, bl31_raw))
        atf_skip = ATF_UPSTREAM_MAGIC_SIZE;
#endif

#ifdef TEE_STAGE_ATF
    // tee_verify_image() moves atf_addr past the MTEE header to the entry
    // point, so copy from wherever it ended up rather than from the start of
    // the staging buffer. A raw image has no header and stays put.
    memcpy((void *)*addr, (const void *)(atf_addr + atf_skip), *size - atf_skip);
    memset((void *)stage_addr, 0, *size);
#else
    *addr = atf_addr + atf_skip;
#endif

    next_offset = PART_HDR_SIZE + *size;

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
#ifdef TEE_SECMEM_START_ADDR
        // A non-zero start means the header carried a size and the Preloader
        // already did the bookkeeping for us, so leave it alone.
        if (!readl(TEE_SECMEM_START_ADDR))
            tee_set_secmem(tee_addr, TEE_MEM_SIZE);
#else
        tee_set_secmem(tee_addr, TEE_MEM_SIZE);
#endif
    } else if (ret) {
        return ret;
    } else if (bl31_raw) {
        // If BL31 lacked an MTEE header but BL32 didn't, then
        // something seriously weird is going on.
        return -1;
    }

#ifdef ATF_HANDOFF_ADDR
    // A mainline BL31 ignores tee_set_entry(), it only learns about BL32
    // through the bl_params chain we build in bldr_jump64().
    if (bl31_raw)
        atf_note_tee(tee_addr);
#endif

    tee_set_entry(tee_addr);
    return 0;
}
