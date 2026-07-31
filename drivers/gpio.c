#include <mmio.h>
#include <platform.h>

#include <drivers/gpio.h>

#define GPIO_DIR_BASE       (GPIO_BASE + GPIO_DIR_OFFSET)
#define GPIO_PULLEN_BASE    (GPIO_BASE + GPIO_PULLEN_OFFSET)
#define GPIO_PULLSEL_BASE   (GPIO_BASE + GPIO_PULLSEL_OFFSET)
#define GPIO_DOUT_BASE      (GPIO_BASE + GPIO_DOUT_OFFSET)
#define GPIO_DIN_BASE       (GPIO_BASE + GPIO_DIN_OFFSET)
#define GPIO_MODE_BASE      (GPIO_BASE + GPIO_MODE_OFFSET)
#define GPIO_IES_BASE       (GPIO_BASE + GPIO_IES_OFFSET)

#define GPIO_MODE_MASK      ((1u << GPIO_MODE_BITS) - 1)

#define gpio_bank(base, pin) \
    ((base) + ((pin) / GPIO_PINS_PER_REG) * GPIO_REG_STRIDE)

#define gpio_bit(pin) \
    (1u << ((pin) % GPIO_PINS_PER_REG))

static const struct gpio_ies_group *gpio_ies_lookup(uint32_t pin)
{
    uint32_t i;

    for (i = 0; i < GPIO_IES_MAP_LEN; i++) {
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

    for (i = 0; i < GPIO_SPEC_PUPD_LEN; i++) {
        if (pin < gpio_spec_pupd[i].start || pin > gpio_spec_pupd[i].end)
            continue;

        base = GPIO_BASE + gpio_spec_pupd[i].off;
        pupd = gpio_spec_pupd[i].pupd0 + 4 * (pin - gpio_spec_pupd[i].start);

        writel(1u << pupd, base + (up ? GPIO_RST : GPIO_SET));
        writel(1u << (pupd - 2), base + (enable ? GPIO_SET : GPIO_RST));
        writel(1u << (pupd - 1), base + GPIO_RST);
        return GPIO_OK;
    }

    // MSDC pads not supported.
    if (gpio_pupd_is_msdc(pin))
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
