#pragma once

#include <stdint.h>

#include "device.h"

struct bldr_command_handler;
typedef uint8_t (*bldr_cmd_handler_t)(struct bldr_command_handler *handler,
                                      void *cmd, void *comm);
struct bldr_command_handler {
    void *priv;
    uint32_t attr;
    bldr_cmd_handler_t cb;
};

static int (*const boot_device_init)(void) = (void *)(BDEV_INIT_ADDR | 1);

static int (*const bldr_load_part)(char *name, void *bdev, uint32_t *addr,
                                   uint32_t *size) =
        (void *)(BLDR_LOAD_PART_ADDR | 1);

static int (*const usb_handshake)(struct bldr_command_handler *handler) =
        (void *)(USB_HANDSHAKE_ADDR | 1);
