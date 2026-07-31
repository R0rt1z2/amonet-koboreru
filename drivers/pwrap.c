#include <stdint.h>

#include <mmio.h>

#include <drivers/pwrap.h>

#define PWRAP_WACS2_CMD    (PWRAP_BASE + 0xA0)
#define PWRAP_WACS2_RDATA  (PWRAP_BASE + 0xA4)
#define PWRAP_WACS2_VLDCLR (PWRAP_BASE + 0xA8)

#define WACS2_FSM_IDLE     0x00
#define WACS2_FSM_WFVLDCLR 0x06
#define WACS2_FSM_MASK     (0x07 << 16)
#define WACS2_FSM_SHIFT    16

#define WACS2_CMD_WRITE (1U << 31)

static uint32_t pwrap_fsm(void)
{
    return (readl(PWRAP_WACS2_RDATA) & WACS2_FSM_MASK) >> WACS2_FSM_SHIFT;
}

static void pwrap_wait_idle(void)
{
    uint32_t fsm;
    do {
        fsm = pwrap_fsm();
    } while (fsm != WACS2_FSM_IDLE && fsm != WACS2_FSM_WFVLDCLR);

    if (fsm == WACS2_FSM_WFVLDCLR)
        writel(1, PWRAP_WACS2_VLDCLR);
}

uint16_t pwrap_read(uint16_t addr)
{
    pwrap_wait_idle();
    writel(((uint32_t)(addr >> 1) << 16), PWRAP_WACS2_CMD);

    do {} while (pwrap_fsm() != WACS2_FSM_WFVLDCLR);

    uint32_t rdata = readl(PWRAP_WACS2_RDATA);
    writel(1, PWRAP_WACS2_VLDCLR);

    return (uint16_t)(rdata & 0xFFFF);
}

void pwrap_write(uint16_t addr, uint16_t data)
{
    pwrap_wait_idle();
    writel(WACS2_CMD_WRITE | ((uint32_t)(addr >> 1) << 16) | data,
           PWRAP_WACS2_CMD);
}
