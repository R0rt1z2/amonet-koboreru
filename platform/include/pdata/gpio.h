#pragma once

struct gpio_ies_group {
    uint8_t start;
    uint8_t end;
    uint8_t reg;
    uint8_t bit;
};

struct gpio_pupd_group {
    uint8_t start;
    uint8_t end;
    /* absolute offset from GPIO_BASE */
    uint16_t off;
    /* pupd bit of `start`; +4 per pin, r1=pupd-1, r0=pupd-2 */
    uint8_t pupd0;
};
