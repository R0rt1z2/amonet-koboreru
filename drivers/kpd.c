#include <stdbool.h>
#include <stdint.h>

#include <mmio.h>

#include <drivers/gpio.h>
#include <drivers/kpd.h>
#include <drivers/timer.h>

#define KPD_MEM1     (KPD_BASE + 0x0004)
#define KPD_DEBOUNCE (KPD_BASE + 0x0018)
#define KPD_SEL      (KPD_BASE + 0x0020)

#define KPD_DEBOUNCE_MASK   GENMASK(13, 0)
#define KPD_DEBOUNCE_MAX_MS 256

#define KPD_SEL_DOUBLE_KP  BIT(0)
#define KPD_SEL_ROW        GENMASK(9, 4)
#define KPD_SEL_COL        GENMASK(15, 10)
#define KPD_SEL_ROWMASK(r) GENMASK((r) + 3, 4)
#define KPD_SEL_COLMASK(c) GENMASK((c) + 9, 10)

#define KPD_PIN_KPCOL0 36
#define KPD_PIN_FUNC   1

#define KPD_MAX_ROWS 3
#define KPD_MAX_COLS 3

void kpd_init(uint32_t rows, uint32_t cols, uint32_t debounce_ms)
{
    uint32_t col, val;

    if (!rows || rows > KPD_MAX_ROWS || !cols || cols > KPD_MAX_COLS)
        return;

    if (debounce_ms > KPD_DEBOUNCE_MAX_MS)
        debounce_ms = KPD_DEBOUNCE_MAX_MS;

    for (col = 0; col < cols; col++) {
        mt_set_gpio_mode(KPD_PIN_KPCOL0 + col, KPD_PIN_FUNC);
        mt_set_gpio_dir(KPD_PIN_KPCOL0 + col, GPIO_DIR_IN);
        mt_set_gpio_ies(KPD_PIN_KPCOL0 + col, GPIO_IES_ENABLE);
        mt_set_gpio_pull(KPD_PIN_KPCOL0 + col, GPIO_PULL_ENABLE, GPIO_PULL_UP);
    }

    writel((debounce_ms << 5) & KPD_DEBOUNCE_MASK, KPD_DEBOUNCE);

    val = readl(KPD_SEL);
    val &= ~(KPD_SEL_ROW | KPD_SEL_COL | KPD_SEL_DOUBLE_KP);
    val |= KPD_SEL_ROWMASK(rows) | KPD_SEL_COLMASK(cols);
    writel(val, KPD_SEL);

    mdelay(debounce_ms + 10);
}

bool kpd_key_pressed(uint32_t key)
{
    if (key > KPD_KEY_MAX)
        return false;

    // The status bits are active low, a cleared bit means the key is down.
    return !(readl(KPD_MEM1 + (key / KPD_KEYS_PER_MEM) * 4)
             & BIT(key % KPD_KEYS_PER_MEM));
}

