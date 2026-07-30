#include <mmio.h>

#include <drivers/wdt.h>

#define WDT_MODE_KEY    0x22000000
#define WDT_MODE_EN     BIT(0)
#define WDT_RESTART_KEY 0x1971
#define WDT_SWRST_KEY   0x1209

#define WDT_MODE_REG     (WDT_BASE + 0x00)
#define WDT_RESTART_REG  (WDT_BASE + 0x08)
#define WDT_SWRST_REG    (WDT_BASE + 0x14)
#define WDT_SWSYSRST_REG (WDT_BASE + 0x18)

void mtk_wdt_kick(void)
{
    writel(WDT_RESTART_KEY, WDT_RESTART_REG);
}

void mtk_wdt_disable(void)
{
    clrsetbits(WDT_MODE_REG, WDT_MODE_EN, WDT_MODE_KEY);
}

void mtk_wdt_reset(void)
{
    writel(WDT_RESTART_KEY, WDT_SWSYSRST_REG);
    writel(WDT_MODE_KEY | WDT_MODE_EN, WDT_MODE_REG);
    writel(WDT_SWRST_KEY, WDT_SWRST_REG);
}