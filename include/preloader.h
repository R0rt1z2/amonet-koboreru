#pragma once

#include <stdint.h>

#include "device.h"

static int (*const boot_device_init)(void) = (void *)(BDEV_INIT_ADDR | 1);

static int (*const bldr_load_part)(char *name, void *bdev, uint32_t *addr,
                                   uint32_t *size) =
        (void *)(BLDR_LOAD_PART_ADDR | 1);
