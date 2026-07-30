#pragma once

#include <stdint.h>

void rtc_init(void);
void rtc_set_fastboot(int enable);
void rtc_set_recovery(int enable);