#pragma once

#include <stdint.h>

#include "device.h"

#define PART_HEADER_DEFAULT_ADDR 0xFFFFFFFF
#define LOAD_ADDR_MODE_BACKWARD  0

struct part_hdr {
    uint32_t magic;
    uint32_t dsize;
    char     name[32];
    uint32_t maddr;
    uint32_t mode;
};

typedef char part_hdr_layout_check[
    (__builtin_offsetof(struct part_hdr, maddr) == 0x28 &&
     __builtin_offsetof(struct part_hdr, mode)  == 0x2C) ? 1 : -1];

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

#ifdef WDT_KICK_ADDR
static void (*const platform_wdt_kick)(void) = (void *)(WDT_KICK_ADDR | 1);
#endif

#ifdef SAFE_MODE_ADDR
static void (*const platform_safe_mode)(int enable, uint32_t timeout_ms) =
        (void *)(SAFE_MODE_ADDR | 1);
#endif

#ifdef TEE_GET_LOAD_ADDR_ADDR
static uint32_t (*const tee_get_load_addr)(uint32_t size) =
        (void *)(TEE_GET_LOAD_ADDR_ADDR | 1);
#endif

#ifdef TRUSTZONE_JUMP_ADDR
static void (*const trustzone_jump)(uint32_t addr, uint32_t arg1, uint32_t arg2) =
        (void *)(TRUSTZONE_JUMP_ADDR | 1);
#endif
