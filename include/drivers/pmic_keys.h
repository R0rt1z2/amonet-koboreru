#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PMIC_KEY_POWER 0
#define PMIC_KEY_HOME  1

bool pmic_key_pressed(uint32_t key);
