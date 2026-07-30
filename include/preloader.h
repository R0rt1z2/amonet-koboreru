#pragma once

#include "device.h"

typedef int (*boot_device_init_fn)(void);
static inline int boot_device_init(void)
{
    boot_device_init_fn fn = (boot_device_init_fn)(uintptr_t)(BDEV_INIT_ADDR | 1);
    return fn();
}

typedef int (*bldr_load_part_fn)(char *name, void *bdev, uint32_t *addr, uint32_t *size);
static inline int bldr_load_part(char *name, void *bdev, uint32_t *addr, uint32_t *size)
{
    bldr_load_part_fn fn = (bldr_load_part_fn)(uintptr_t)(BLDR_LOAD_PART_ADDR | 1);
    return fn(name, bdev, addr, size);
}
