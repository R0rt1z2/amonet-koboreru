#pragma once

#include <stdint.h>

#define TEE_IMAGE_HDR_SIZE  0x240
#define TEE_IMAGE_SIG_SIZE  0x100

#define TEE_IMAGE_MAGIC0     0x20544545 /* "EET " */
#define TEE_IMAGE_MAGIC1     0x204D544B /* "KTM " */

struct tee_image_hdr {
    uint32_t magic[2];
    uint32_t hdrsize;
    uint32_t version;
    uint32_t flags;
    uint32_t datalen;
    uint32_t datalen2;
    uint8_t keyseed[16];
    uint8_t ivseed[16];
    uint8_t reserved[0x100];
    uint8_t signature[TEE_IMAGE_SIG_SIZE];
    uint32_t secmem_size;
};

typedef char tee_image_hdr_size_check[
    (sizeof(struct tee_image_hdr) == TEE_IMAGE_HDR_SIZE) ? 1 : -1];

typedef char tee_image_hdr_layout_check[
    (__builtin_offsetof(struct tee_image_hdr, hdrsize)  == 0x008 &&
     __builtin_offsetof(struct tee_image_hdr, datalen2) == 0x018 &&
     __builtin_offsetof(struct tee_image_hdr, keyseed)  == 0x01C &&
     __builtin_offsetof(struct tee_image_hdr, ivseed)   == 0x02C &&
     __builtin_offsetof(struct tee_image_hdr, signature) == 0x13C &&
     __builtin_offsetof(struct tee_image_hdr, secmem_size) == 0x23C) ? 1 : -1];

int bldr_load_tee_part(char *name, void *bdev, uint32_t *addr, uint32_t offset, uint32_t *size);
