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

static void* (*const part_get)(char *name) = (void *)(PART_GET_ADDR | 1);

static int (*const part_load)(void *bdev, void *part, uint32_t *addr, uint32_t offset,
        uint32_t* size) = (void *)(PART_LOAD_ADDR | 1);

static inline int bldr_load_part(char *name, void *bdev, uint32_t *addr, uint32_t *size)
{
    void *part = part_get(name);
    if (!part)
        return -1;

    return part_load(bdev, part, addr, 0, size);
}

static int (*const usb_handshake)(struct bldr_command_handler *handler) =
        (void *)(USB_HANDSHAKE_ADDR | 1);

#ifdef USB_CABLE_IN_ADDR
static int (*const usb_cable_in)(void) = (void *)(USB_CABLE_IN_ADDR | 1);
#else
static inline int usb_cable_in(void) { return 1; }
#endif

static int (*const mtee_verify_decrypt)(uint32_t *addr, uint32_t secmem_size, const uint8_t *pubk) =
        (void *)(MTEE_VERIFY_DECRYPT_ADDR | 1);

static void (*const tee_set_entry)(uint32_t addr) =
        (void *)(TEE_SET_ENTRY_ADDR | 1);
