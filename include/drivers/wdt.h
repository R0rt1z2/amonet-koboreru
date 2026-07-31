#pragma once

#include <platform.h>

#include <inttypes.h>

#ifndef WDT_BASE
#error "WDT_BASE must be defined"
#endif

void mtk_wdt_kick(void);
void mtk_wdt_disable(void);
void mtk_wdt_reset(void);