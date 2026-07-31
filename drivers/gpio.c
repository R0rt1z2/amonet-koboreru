#include <mmio.h>

#include <drivers/gpio.h>

#define GPIO_REG_STRIDE     0x10
#define GPIO_VAL            0x0
#define GPIO_SET            0x4
#define GPIO_RST            0x8

#define GPIO_DIR_BASE       (GPIO_BASE + 0x000)
#define GPIO_PULLEN_BASE    (GPIO_BASE + 0x100)
#define GPIO_PULLSEL_BASE   (GPIO_BASE + 0x200)
#define GPIO_DOUT_BASE      (GPIO_BASE + 0x400)
#define GPIO_DIN_BASE       (GPIO_BASE + 0x500)
#define GPIO_MODE_BASE      (GPIO_BASE + 0x600)
#define GPIO_IES_BASE       (GPIO_BASE + 0x900)

#define GPIO_PINS_PER_REG   16
#define GPIO_MODE_PER_REG   5
#define GPIO_MODE_BITS      3
#define GPIO_MODE_MASK      0x7

#define gpio_bank(base, pin) \
    ((base) + ((pin) / GPIO_PINS_PER_REG) * GPIO_REG_STRIDE)

#define gpio_bit(pin) \
    (1u << ((pin) % GPIO_PINS_PER_REG))

#define GPIO_SPEC_PUPD_N \
    (sizeof(gpio_spec_pupd) / sizeof(gpio_spec_pupd[0]))

#define GPIO_IES_GROUPS \
    (sizeof(gpio_ies_map) / sizeof(gpio_ies_map[0]))

#define IES_UNSUPPORTED 0xff

struct gpio_ies_group {
    uint8_t start;
    uint8_t end;
    uint8_t reg;
    uint8_t bit;
};

struct gpio_pupd_group {
    uint8_t start;
    uint8_t end;
    uint16_t off;
};

static const struct gpio_ies_group gpio_ies_map[] = {
    {   0,   9, 0,  0 }, {  10,  13, 0,  1 }, {  14,  28, 0,  2 },
    {  29,  32, 0,  3 }, {  33,  33, 1, 11 }, {  34,  38, 0, 10 },
    {  39,  42, 0, 11 }, {  43,  45, 0, 12 }, {  46,  49, 0, 13 },
    {  50,  52, 1, 10 }, {  53,  56, 0, 14 }, {  57,  58, 1,  0 },
    {  59,  65, 1,  2 }, {  66,  71, 1,  3 }, {  72,  74, 1,  4 },
    {  75,  76, 0, 15 }, {  77,  78, 1,  1 }, {  79,  82, 1,  5 },
    {  83,  84, 1,  6 },
    {  85,  90, IES_UNSUPPORTED, 0 },   /* MSDC2      */
    {  91, 100, IES_UNSUPPORTED, 0 },   /* TDP/TCP    */
    { 101, 116, IES_UNSUPPORTED, 0 },   /* GPI        */
    { 117, 120, 1,  7 },
    { 121, 126, IES_UNSUPPORTED, 0 },   /* MSDC1      */
    { 127, 137, IES_UNSUPPORTED, 0 },   /* MSDC0      */
    { 138, 141, 1,  9 },
    { 142, 142, 0, 13 },
    { 143, 154, IES_UNSUPPORTED, 0 },   /* MSDC3      */
};

static const struct gpio_pupd_group gpio_spec_pupd[] = {
    {  33,  35, 0xd90 },    /* KPROW0..2  */
    {  36,  38, 0xda0 },    /* KPCOL0..2  */
    {  46,  49, 0xdb0 },    /* EINT14..17 */
    { 142, 142, 0xdc0 },    /* EINT21     */
};

static const struct gpio_ies_group *gpio_ies_lookup(uint32_t pin)
{
    uint32_t i;

    for (i = 0; i < GPIO_IES_GROUPS; i++) {
        if (pin >= gpio_ies_map[i].start && pin <= gpio_ies_map[i].end)
            return gpio_ies_map[i].reg == IES_UNSUPPORTED
                       ? (const struct gpio_ies_group *)0
                       : &gpio_ies_map[i];
    }
    return (const struct gpio_ies_group *)0;
}

static inline uint32_t gpio_pin_decrypt(uint32_t pin)
{
    return pin & 0x7fffffff;
}

static inline void gpio_set_bit(uint32_t base, uint32_t pin, uint32_t val)
{
    writel(gpio_bit(pin), gpio_bank(base, pin) + (val ? GPIO_SET : GPIO_RST));
}

static inline int gpio_get_bit(uint32_t base, uint32_t pin)
{
    return (readl(gpio_bank(base, pin) + GPIO_VAL) >> (pin % GPIO_PINS_PER_REG)) & 1;
}

int mt_get_gpio_in(uint32_t pin)
{
    pin = gpio_pin_decrypt(pin);

    if (pin >= GPIO_MAX_PIN)
        return GPIO_ERR_INVALID;

    return gpio_get_bit(GPIO_DIN_BASE, pin);
}

int mt_get_gpio_out(uint32_t pin)
{
    pin = gpio_pin_decrypt(pin);

    if (pin >= GPIO_MAX_PIN)
        return GPIO_ERR_INVALID;

    return gpio_get_bit(GPIO_DOUT_BASE, pin);
}

int mt_get_gpio_dir(uint32_t pin)
{
    pin = gpio_pin_decrypt(pin);

    if (pin >= GPIO_MAX_PIN)
        return GPIO_ERR_INVALID;

    return gpio_get_bit(GPIO_DIR_BASE, pin);
}

int mt_set_gpio_out(uint32_t pin, uint32_t val)
{
    pin = gpio_pin_decrypt(pin);

    if (pin >= GPIO_MAX_PIN || val > GPIO_OUT_ONE)
        return GPIO_ERR_INVALID;

    gpio_set_bit(GPIO_DOUT_BASE, pin, val);
    return GPIO_OK;
}

int mt_set_gpio_dir(uint32_t pin, uint32_t dir)
{
    pin = gpio_pin_decrypt(pin);

    if (pin >= GPIO_MAX_PIN || dir > GPIO_DIR_OUT)
        return GPIO_ERR_INVALID;

    gpio_set_bit(GPIO_DIR_BASE, pin, dir);
    return GPIO_OK;
}

int mt_set_gpio_pull(uint32_t pin, uint32_t enable, uint32_t up)
{
    uint32_t i, base, pupd;

    pin = gpio_pin_decrypt(pin);

    if (pin >= GPIO_MAX_PIN || enable > 1 || up > 1)
        return GPIO_ERR_INVALID;

    for (i = 0; i < GPIO_SPEC_PUPD_N; i++) {
        if (pin < gpio_spec_pupd[i].start || pin > gpio_spec_pupd[i].end)
            continue;

        base = GPIO_BASE + gpio_spec_pupd[i].off;
        pupd = 2 + 4 * (pin - gpio_spec_pupd[i].start);

        writel(1u << pupd, base + (up ? GPIO_RST : GPIO_SET));
        writel(1u << (pupd - 2), base + (enable ? GPIO_SET : GPIO_RST));
        writel(1u << (pupd - 1), base + GPIO_RST);
        return GPIO_OK;
    }

    // MSDC pads not supported.
    if ((pin >= 85 && pin <= 90) || (pin >= 121 && pin <= 137) || pin >= 143)
        return GPIO_ERR_UNSUPPORTED;

    gpio_set_bit(GPIO_PULLSEL_BASE, pin, up);
    gpio_set_bit(GPIO_PULLEN_BASE, pin, enable);
    return GPIO_OK;
}

int mt_get_gpio_mode(uint32_t pin)
{
    uint32_t reg, shift;

    pin = gpio_pin_decrypt(pin);

    if (pin >= GPIO_MAX_PIN)
        return GPIO_ERR_INVALID;

    reg = GPIO_MODE_BASE + (pin / GPIO_MODE_PER_REG) * GPIO_REG_STRIDE;
    shift = (pin % GPIO_MODE_PER_REG) * GPIO_MODE_BITS;

    return (readl(reg + GPIO_VAL) >> shift) & GPIO_MODE_MASK;
}

int mt_set_gpio_mode(uint32_t pin, uint32_t mode)
{
    uint32_t reg, shift, val;

    pin = gpio_pin_decrypt(pin);

    if (pin >= GPIO_MAX_PIN || mode > GPIO_MODE_MAX)
        return GPIO_ERR_INVALID;

    reg = GPIO_MODE_BASE + (pin / GPIO_MODE_PER_REG) * GPIO_REG_STRIDE;
    shift = (pin % GPIO_MODE_PER_REG) * GPIO_MODE_BITS;

    val = readl(reg + GPIO_VAL);
    val &= ~(GPIO_MODE_MASK << shift);
    val |= mode << shift;
    writel(val, reg + GPIO_VAL);

    return GPIO_OK;
}

int mt_set_gpio_ies(uint32_t pin, uint32_t enable)
{
    const struct gpio_ies_group *g;

    pin = gpio_pin_decrypt(pin);

    if (pin >= GPIO_MAX_PIN || enable > GPIO_IES_ENABLE)
        return GPIO_ERR_INVALID;

    g = gpio_ies_lookup(pin);
    if (!g)
        return GPIO_ERR_UNSUPPORTED;

    writel(1u << g->bit, GPIO_IES_BASE + g->reg * GPIO_REG_STRIDE
                             + (enable ? GPIO_SET : GPIO_RST));
    return GPIO_OK;
}

int mt_get_gpio_ies(uint32_t pin)
{
    const struct gpio_ies_group *g;

    pin = gpio_pin_decrypt(pin);

    if (pin >= GPIO_MAX_PIN)
        return GPIO_ERR_INVALID;

    g = gpio_ies_lookup(pin);
    if (!g)
        return GPIO_ERR_UNSUPPORTED;

    return (readl(GPIO_IES_BASE + g->reg * GPIO_REG_STRIDE + GPIO_VAL)
            >> g->bit) & 1;
}
