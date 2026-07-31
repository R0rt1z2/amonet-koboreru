#pragma once

#include <platform.h>

#include <stdbool.h>
#include <stdint.h>

#ifndef KPD_BASE
#error "KPD_BASE must be defined"
#endif

#define KPD_KEYS_PER_MEM 16
#define KPD_NUM_MEMS     5
#define KPD_KEY_MAX      (KPD_NUM_MEMS * KPD_KEYS_PER_MEM - 1)

#define KPD_KEY(row, col) ((row) * 9 + (col))

void kpd_init(uint32_t rows, uint32_t cols, uint32_t debounce_ms);
bool kpd_key_pressed(uint32_t key);
