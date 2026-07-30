#include <stdint.h>

#include <debug.h>
#include <mmio.h>

#include <drivers/devinfo.h>

#define DEVINFO_MAX_INDEX 0x29

#define BROM_CMD_DIS_INDEX  6
#define BROM_CMD_DIS_OFFSET 8

static const uint32_t g_devinfo_addrs[DEVINFO_MAX_INDEX] = {
    0x10206020, 0x10206030, 0x10206038, 0x10206040,
    0x10206044, 0x10206048, 0x10206060, 0x10206100,
    0x10206104, 0x10206108, 0x10206120, 0x10206130,
    0x10206140, 0x10206144, 0x10206170, 0x10206174,
    0x10206178, 0x1020617C, 0x10206180, 0x10206184,
    0x08000000, 0x10206188, 0x102061B0, 0x102061B4,
    0x102061B8, 0x102061BC, 0x102061C0, 0x102061C4,
    0x102061C8, 0x102061CC, 0x1020604C, 0x10206050,
    0x10206054, 0x10206090, 0x10206094, 0x10206098,
    0x1020609C, 0x102060A0, 0x102060A4, 0x102060A8,
    0x102060AC,
};

uint32_t get_devinfo_with_index(uint32_t index)
{
    if (index >= DEVINFO_MAX_INDEX) {
        printf("ERROR: Invalid devinfo index %lu\n", index);
        return 0xFFFFFFFF;
    }

    return readl(g_devinfo_addrs[index]);
}

int is_brom_cmd_disabled(void)
{
    uint32_t val = get_devinfo_with_index(BROM_CMD_DIS_INDEX);
    if (val == 0xFFFFFFFF)
        return -1;

    return (val >> BROM_CMD_DIS_OFFSET) & 1;
}