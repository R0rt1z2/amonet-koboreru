#pragma once

#include <inttypes.h>

unsigned int strlen(const char *s);
int streq(const char *a, const char *b);
void strnarrow(const uint16_t *src, char *dst, int max);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, unsigned int n);