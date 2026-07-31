#pragma once

#include <platform.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0

#include <nanoprintf.h>

#ifndef UART_BASE
#error "UART_BASE must be defined"
#endif

int printf(const char *fmt, ...) NPF_PRINTF_ATTR(1, 2);
void hexdump(const void *data, uint32_t size);
