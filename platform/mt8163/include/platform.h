#pragma once

#include <stdint.h>

#include <pdata/gpio.h>
#include <pdata/pmic_keys.h>

#define PLATFORM_NAME "mt8163"

/* MMIO */
#define UART_BASE       0x11002000
#define WDT_BASE        0x10007000
#define APXGPT_BASE     0x10008000
#define PWRAP_BASE      0x1000D000
#define GPIO_BASE       0x10005000
#define KPD_BASE        0x10010000

/* DEVINFO */
#define DEVINFO_MAX_INDEX 0x29

#define BROM_CMD_DIS_INDEX  6
#define BROM_CMD_DIS_OFFSET 8

extern const uint32_t g_devinfo_addrs[DEVINFO_MAX_INDEX];

/* GPIO */
#define GPIO_REG_STRIDE     0x10
#define GPIO_VAL            0x0
#define GPIO_SET            0x4
#define GPIO_RST            0x8

#define GPIO_DIR_OFFSET     0x000
#define GPIO_PULLEN_OFFSET  0x100
#define GPIO_PULLSEL_OFFSET 0x200
#define GPIO_DOUT_OFFSET    0x400
#define GPIO_DIN_OFFSET     0x500
#define GPIO_MODE_OFFSET    0x600
#define GPIO_IES_OFFSET     0x900

#define GPIO_MAX_PIN        0x9b
#define GPIO_PINS_PER_REG   16
#define GPIO_MODE_PER_REG   5
#define GPIO_MODE_BITS      3

#define IES_UNSUPPORTED     0xff

#define GPIO_IES_MAP_LEN    28
#define GPIO_SPEC_PUPD_LEN   4

extern const struct gpio_ies_group gpio_ies_map[];
extern const struct gpio_pupd_group gpio_spec_pupd[];

#define gpio_pupd_is_msdc(pin) \
    (((pin) >= 85 && (pin) <= 90) || ((pin) >= 121 && (pin) <= 137) || (pin) >= 143)

/* KEYPAD */
#define KPD_PIN_KPCOL0 36
#define KPD_PIN_FUNC   1

#define KPD_MAX_ROWS 3
#define KPD_MAX_COLS 3

/* PMIC KEYS */
#define PMIC_KEY_NR 2
extern const struct pmic_keys_regs g_pmic_keys_regs[PMIC_KEY_NR];

/* USB DESCRIPTORS */
#define USB_MANUFACTURER       "PWNED"
#define USB_PRODUCT_NAME       "MT8163 Preloader"
#define USB_CONFIGURATION_STR  "USB CDC ACM for preloader"
#define USB_DATA_INTERFACE_STR "CDC ACM Data Interface"
#define USB_COMM_INTERFACE_STR "CDC ACM Communication Interface"
#define USB_ISERIAL_STR         "MT-0123456789ABCDEF"
