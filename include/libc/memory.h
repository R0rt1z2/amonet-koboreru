#pragma once

#include <inttypes.h>

int memcmp(const void *a, const void *b, unsigned int n);
void *memcpy(void *dst, const void *src, unsigned int n);
void *memset(void *dst, int c, unsigned int n);