#pragma once

#include <patch.h>

#define DEVICE_NAME "crown"

#define PAYLOAD_ADDR 0x00101000
#define BSS_START    0x00102180
#define DRAM_BUF     0x4BE00000
#define BDEV_ADDR    0x00102DC0

#define PRELOADER_ENTRY    0x00201018
#define PRELOADER_BOOT_ARG 0x00201000

#define DAA_GATE           0x0021B258
#define VERIFY_LK          0x0020FD50
#define TEE_PARTITION_LIT  0x00209794
#define TEE_PARTITION_TEE2 0x00014EB7
#define RPMB_MAGIC_CHECK   0x002094C8
#define RPMB_MAGIC_IGNORED 0xBF0058E6

#define DEVICE_PATCHES                             \
    { DAA_GATE, THUMB_RET_ZERO },                  \
    { VERIFY_LK, THUMB_RET_ZERO },                 \
    { TEE_PARTITION_LIT, TEE_PARTITION_TEE2 },     \
    { RPMB_MAGIC_CHECK, RPMB_MAGIC_IGNORED },
