#include <platform.h>
#include <mmio.h>

#include <drivers/pwrap.h>
#include <drivers/rtc.h>

#define RTC_BBPU       (RTC_BASE + 0x0000)
#define RTC_BBPU_CBUSY (1U << 6)
#define RTC_PDN1       (RTC_BASE + 0x002C)
#define RTC_PROT       (RTC_BASE + 0x0036)
#define RTC_WRTGR      (RTC_BASE + 0x003C)

#define RTC_PDN1_FAST_BOOT (1U << 13)
#define RTC_PDN1_FAC_RESET (1U << 4)

#define RTC_PROT_UNLOCK1 0x586A
#define RTC_PROT_UNLOCK2 0x9136

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