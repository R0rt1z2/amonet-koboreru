#include <mmio.h>

#include <drivers/rtc.h>

#define PWRAP_WACS2_CMD    (PWRAP_BASE + 0xA0)
#define PWRAP_WACS2_RDATA  (PWRAP_BASE + 0xA4)
#define PWRAP_WACS2_VLDCLR (PWRAP_BASE + 0xA8)

#define WACS2_FSM_IDLE     0x00
#define WACS2_FSM_WFVLDCLR 0x06
#define WACS2_FSM_MASK     (0x07 << 16)
#define WACS2_FSM_SHIFT    16

#define RTC_BBPU       (RTC_BASE + 0x0000)
#define RTC_BBPU_CBUSY (1U << 6)
#define RTC_PDN1       (RTC_BASE + 0x002C)
#define RTC_PROT       (RTC_BASE + 0x0036)
#define RTC_WRTGR      (RTC_BASE + 0x003C)

#define RTC_PDN1_FAST_BOOT (1U << 13)
#define RTC_PDN1_FAC_RESET (1U << 4)

#define RTC_PROT_UNLOCK1 0x586A
#define RTC_PROT_UNLOCK2 0x9136

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

static uint16_t pwrap_read(uint16_t addr)
{
    pwrap_wait_idle();
    writel(((uint32_t)(addr >> 1) << 16), PWRAP_WACS2_CMD);

    do {} while (pwrap_fsm() != WACS2_FSM_WFVLDCLR);

    uint32_t rdata = readl(PWRAP_WACS2_RDATA);
    writel(1, PWRAP_WACS2_VLDCLR);

    return (uint16_t)(rdata & 0xFFFF);
}

static void pwrap_write(uint16_t addr, uint16_t data)
{
    pwrap_wait_idle();
    writel((1U << 31) | ((uint32_t)(addr >> 1) << 16) | data, PWRAP_WACS2_CMD);
}

static void rtc_busy_wait(void)
{
    while (pwrap_read(RTC_BBPU) & RTC_BBPU_CBUSY)
        ;
}

static void rtc_write_trigger(void)
{
    pwrap_write(RTC_WRTGR, 1);
    rtc_busy_wait();
}

static void rtc_writeif_unlock(void)
{
    pwrap_write(RTC_PROT, RTC_PROT_UNLOCK1);
    rtc_write_trigger();
    pwrap_write(RTC_PROT, RTC_PROT_UNLOCK2);
    rtc_write_trigger();
}

void rtc_init(void)
{
    rtc_writeif_unlock();
}

void rtc_set_fastboot(int enable)
{
    uint16_t pdn1;

    rtc_writeif_unlock();
    pdn1 = pwrap_read(RTC_PDN1);
    if (enable)
        pdn1 |= RTC_PDN1_FAST_BOOT;
    else
        pdn1 &= ~RTC_PDN1_FAST_BOOT;
    pwrap_write(RTC_PDN1, pdn1);
    rtc_write_trigger();
}

void rtc_set_recovery(int enable)
{
    uint16_t pdn1;

    rtc_writeif_unlock();
    pdn1 = pwrap_read(RTC_PDN1);
    if (enable)
        pdn1 |= RTC_PDN1_FAC_RESET;
    else
        pdn1 &= ~RTC_PDN1_FAC_RESET;
    pwrap_write(RTC_PDN1, pdn1);
    rtc_write_trigger();
}