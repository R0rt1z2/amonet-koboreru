#include <debug.h>
#include <device.h>
#include <mmio.h>
#include <preloader.h>

#include <libc/memory.h>

#include "atf.h"

#ifdef ATF_HANDOFF_ADDR

#ifndef MP0_RV_ADDR_LO
#error "ATF_HANDOFF_ADDR needs MP0_RV_ADDR_LO from the platform header"
#endif
#if !defined(ATF_LOAD_ADDR) || !defined(WDT_KICK_ADDR) || !defined(SAFE_MODE_ADDR) || \
    !defined(TRUSTZONE_JUMP_ADDR)
#error "ATF_HANDOFF_ADDR needs ATF_LOAD_ADDR, WDT_KICK_ADDR, SAFE_MODE_ADDR and TRUSTZONE_JUMP_ADDR"
#endif

#define ATF_SAFE_MODE_TIMEOUT_MS 5000

#define PARAM_EP           0x01
#define PARAM_IMAGE_BINARY 0x02
#define PARAM_BL_PARAMS    0x05
#define PARAM_VERSION_2    0x02
#define EP_SECURE          0x00
#define EP_NON_SECURE      0x01
#define BL32_IMAGE_ID      4
#define BL33_IMAGE_ID      5

/* SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS) */
#define SPSR_EL2H_MASKED   0x3c9
/* SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS) */
#define SPSR_EL1H_MASKED   0x3c5

typedef struct {
    uint8_t  type;
    uint8_t  version;
    uint16_t size;
    uint32_t attr;
} atf_param_header_t;

typedef struct {
    atf_param_header_t h;
    uint64_t pc;
    uint32_t spsr;
    uint32_t reserved;
    uint64_t args[8];
} atf_entry_point_info_t;

typedef struct {
    atf_param_header_t h;
    uint64_t image_base;
    uint32_t image_size;
    uint32_t image_max_size;
} atf_image_info_t;

typedef struct {
    uint32_t image_id;
    uint32_t reserved;
    uint64_t image_info;
    uint64_t ep_info;
    uint64_t next_params_info;
} atf_params_node_t;

typedef struct {
    atf_param_header_t h;
    uint64_t head;
} atf_params_t;

typedef char atf_abi_layout_check[
    (sizeof(atf_param_header_t)     == 0x08 &&
     sizeof(atf_entry_point_info_t) == 0x58 &&
     sizeof(atf_image_info_t)       == 0x18 &&
     sizeof(atf_params_node_t)      == 0x20 &&
     sizeof(atf_params_t)           == 0x10 &&
     __builtin_offsetof(atf_entry_point_info_t, pc)    == 0x08 &&
     __builtin_offsetof(atf_entry_point_info_t, spsr)  == 0x10 &&
     __builtin_offsetof(atf_entry_point_info_t, args)  == 0x18 &&
     __builtin_offsetof(atf_params_node_t, image_info) == 0x08 &&
     __builtin_offsetof(atf_params_node_t, ep_info)    == 0x10 &&
     __builtin_offsetof(atf_params_t, head)            == 0x08) ? 1 : -1];

typedef struct {
    atf_params_t params;
    atf_params_node_t      bl32_node;
    atf_image_info_t       bl32_image_info;
    atf_entry_point_info_t bl32_ep_info;
    atf_params_node_t      bl33_node;
    atf_image_info_t       bl33_image_info;
    atf_entry_point_info_t bl33_ep_info;
} atf_handoff_t;

typedef struct {
    uint32_t code[4];
    uint64_t params;
    uint64_t entry;
} atf_trampoline_t;

#define ATF_PARAMS_ADDR     (ATF_HANDOFF_ADDR)
#define ATF_TRAMPOLINE_ADDR (ATF_HANDOFF_ADDR + 0x1000)

static atf_handoff_t    *const handoff    = (void *)(uintptr_t)ATF_PARAMS_ADDR;
static atf_trampoline_t *const trampoline = (void *)(uintptr_t)ATF_TRAMPOLINE_ADDR;

static uint8_t upstream;
static uint32_t bl32_entry;

int atf_note_image(uint32_t addr, int raw)
{
    const uint32_t *magic = (const uint32_t *)(uintptr_t)addr;

    bl32_entry = 0;
    upstream = raw && magic[0] == ATF_UPSTREAM_MAGIC0
                   && magic[1] == ATF_UPSTREAM_MAGIC1;
    if (upstream)
        printf("Upstream ATF image detected\n");

    return upstream;
}

void atf_note_tee(uint32_t addr)
{
    bl32_entry = upstream ? addr : 0;
}

/* bldr_jump64(jump_addr, &bootarg, sizeof(boot_arg_t)). */
void bldr_jump64(uint32_t addr, uint32_t arg1, uint32_t arg2)
{
    if (!upstream) {
        trustzone_jump(addr, arg1, arg2);
        __builtin_unreachable();
    }

    platform_wdt_kick();
    platform_safe_mode(1, ATF_SAFE_MODE_TIMEOUT_MS);

    memset(handoff, 0, sizeof(*handoff));

    handoff->params.h.type    = PARAM_BL_PARAMS;
    handoff->params.h.version = PARAM_VERSION_2;
    handoff->params.h.size    = sizeof(handoff->params);
    handoff->params.head      = (uintptr_t)&handoff->bl33_node;

    // BL31 runs BL32 before BL33
    if (bl32_entry) {
        handoff->params.head = (uintptr_t)&handoff->bl32_node;

        handoff->bl32_node.image_id         = BL32_IMAGE_ID;
        handoff->bl32_node.image_info       = (uintptr_t)&handoff->bl32_image_info;
        handoff->bl32_node.ep_info          = (uintptr_t)&handoff->bl32_ep_info;
        handoff->bl32_node.next_params_info = (uintptr_t)&handoff->bl33_node;

        handoff->bl32_image_info.h.type     = PARAM_IMAGE_BINARY;
        handoff->bl32_image_info.h.version  = PARAM_VERSION_2;
        handoff->bl32_image_info.h.size     = sizeof(handoff->bl32_image_info);
        handoff->bl32_image_info.image_base = bl32_entry;

        handoff->bl32_ep_info.h.type    = PARAM_EP;
        handoff->bl32_ep_info.h.version = PARAM_VERSION_2;
        handoff->bl32_ep_info.h.size    = sizeof(handoff->bl32_ep_info);
        handoff->bl32_ep_info.h.attr    = EP_SECURE;
        handoff->bl32_ep_info.pc        = bl32_entry;
        handoff->bl32_ep_info.spsr      = SPSR_EL1H_MASKED;
    }

    handoff->bl33_node.image_id   = BL33_IMAGE_ID;
    handoff->bl33_node.image_info = (uintptr_t)&handoff->bl33_image_info;
    handoff->bl33_node.ep_info    = (uintptr_t)&handoff->bl33_ep_info;

    handoff->bl33_image_info.h.type     = PARAM_IMAGE_BINARY;
    handoff->bl33_image_info.h.version  = PARAM_VERSION_2;
    handoff->bl33_image_info.h.size     = sizeof(handoff->bl33_image_info);
    handoff->bl33_image_info.image_base = addr;

    handoff->bl33_ep_info.h.type    = PARAM_EP;
    handoff->bl33_ep_info.h.version = PARAM_VERSION_2;
    handoff->bl33_ep_info.h.size    = sizeof(handoff->bl33_ep_info);
    handoff->bl33_ep_info.h.attr    = EP_NON_SECURE;
    handoff->bl33_ep_info.pc        = addr;
    handoff->bl33_ep_info.spsr      = SPSR_EL2H_MASKED;
    handoff->bl33_ep_info.args[4]   = arg1;

    memset(trampoline, 0, sizeof(*trampoline));
    trampoline->code[0] = 0x58000080;
    trampoline->code[1] = 0x580000a1;
    trampoline->code[2] = 0xd61f0020;
    trampoline->code[3] = 0xd503201f;
    trampoline->params  = (uintptr_t)&handoff->params;
    trampoline->entry   = ATF_LOAD_ADDR;

    flush_dcache_range(handoff, sizeof(*handoff));
    flush_dcache_range(trampoline, sizeof(*trampoline));
    invalidate_icache();

    writel((uint32_t)(uintptr_t)trampoline, MP0_RV_ADDR_LO);

    __asm__ volatile(
        /* RMR.AA64: come up in AArch64. */
        "mrc p15, 0, r0, c12, c0, 2 \n"
        "orr r0, r0, #1             \n"
        "mcr p15, 0, r0, c12, c0, 2 \n"
        "dsb                        \n"
        "isb                        \n"

        /* RMR.RR: ask for the reset. */
        "mrc p15, 0, r0, c12, c0, 2 \n"
        "orr r0, r0, #2             \n"
        "mcr p15, 0, r0, c12, c0, 2 \n"
        "dsb                        \n"
        "isb                        \n"

        "1: wfi                     \n"
        "b   1b                     \n"
        :
        :
        : "r0", "memory");

    __builtin_unreachable();
}

#endif /* ATF_HANDOFF_ADDR */
