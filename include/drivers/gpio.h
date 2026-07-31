#pragma once

#include <stdint.h>

#include <platform.h>

#define GPIO_OK              0
#define GPIO_ERR_INVALID     (-1)
#define GPIO_ERR_UNSUPPORTED (-2)   /* pad's IES lives in the msdc*_ctrl regs */

#define GPIO_DIR_IN         0
#define GPIO_DIR_OUT        1

#define GPIO_OUT_ZERO       0
#define GPIO_OUT_ONE        1

#define GPIO_PULL_DISABLE   0
#define GPIO_PULL_ENABLE    1

#define GPIO_PULL_DOWN      0
#define GPIO_PULL_UP        1

#define GPIO_MODE_GPIO      0
#define GPIO_MODE_MAX       7

#define GPIO_IES_DISABLE    0
#define GPIO_IES_ENABLE     1

int mt_get_gpio_in(uint32_t pin);
int mt_get_gpio_out(uint32_t pin);
int mt_get_gpio_dir(uint32_t pin);
int mt_get_gpio_mode(uint32_t pin);
int mt_get_gpio_ies(uint32_t pin);

int mt_set_gpio_out(uint32_t pin, uint32_t val);
int mt_set_gpio_dir(uint32_t pin, uint32_t dir);
int mt_set_gpio_mode(uint32_t pin, uint32_t mode);
int mt_set_gpio_ies(uint32_t pin, uint32_t enable);

int mt_set_gpio_pull(uint32_t pin, uint32_t enable, uint32_t up);
