#include <device.h>

#include "arb.h"

#ifdef RPMB_BLOCK0_READ_ADDR

#include <stdint.h>

static int (*const rpmb_block0_read)(void *buf) =
        (void *)(RPMB_BLOCK0_READ_ADDR | 1);
static int (*const rpmb_get_wc)(uint32_t *wc) =
        (void *)(RPMB_GET_WC_ADDR | 1);
static int (*const rpmb_block0_write)(void *buf, uint32_t wc) =
        (void *)(RPMB_BLOCK0_WRITE_ADDR | 1);

#define ARB_PL_VER  4
#define ARB_TEE_VER 5
#define ARB_LK_VER  6
#define ARB_CKSUM   7

static uint16_t arb_checksum(const uint16_t *b)
{
    uint32_t sum = 0;
    int i;

    for (i = 0; i < 7; i++)
        sum += b[i];
    while (sum >> 16)
        sum = (sum >> 16) + (sum & 0xffff);

    return ~(uint16_t)sum;
}

void clear_rpmb_arb(void)
{
    uint8_t block[0x100];
    uint16_t *b = (uint16_t *)block;
    uint32_t wc;

    if (rpmb_block0_read(block))
        return;

    if (!b[ARB_PL_VER] && !b[ARB_TEE_VER] && !b[ARB_LK_VER])
        return;

    b[ARB_PL_VER] = 0;
    b[ARB_TEE_VER] = 0;
    b[ARB_LK_VER] = 0;
    b[ARB_CKSUM] = arb_checksum(b);

    if (rpmb_get_wc(&wc) == 0)
        rpmb_block0_write(block, wc);
}

#endif
