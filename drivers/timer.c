#include <platform.h>
#include <stdint.h>

#include <mmio.h>

#include <drivers/timer.h>

#define GPT4_DAT_REG    (APXGPT_BASE + 0x0048)
#define GPT4_1US_TICK   13
#define GPT4_1MS_TICK   13000

uint32_t gpt4_get_current_tick(void)
{
    return readl(GPT4_DAT_REG);
}

uint32_t gpt4_tick2time_ms(uint32_t tick)
{
    return tick / GPT4_1MS_TICK;
}

void mdelay(unsigned long ms)
{
    uint32_t start = readl(GPT4_DAT_REG);
    uint32_t timeout = ms * GPT4_1MS_TICK;

    while ((readl(GPT4_DAT_REG) - start) < timeout)
        ;
}

void udelay(unsigned long us)
{
    uint32_t start = readl(GPT4_DAT_REG);
    uint32_t timeout = us * GPT4_1US_TICK;

    while ((readl(GPT4_DAT_REG) - start) < timeout)
        ;
}