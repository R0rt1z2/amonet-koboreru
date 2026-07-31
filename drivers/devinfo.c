#include <stdint.h>

#include <debug.h>
#include <mmio.h>

#include <platform.h>

#include <drivers/devinfo.h>

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