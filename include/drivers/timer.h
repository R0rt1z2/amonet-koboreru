#pragma once

#include <stdint.h>

void mdelay(unsigned long ms);
void udelay(unsigned long us);
uint32_t gpt4_get_current_tick(void);
uint32_t gpt4_tick2time_ms(uint32_t tick);