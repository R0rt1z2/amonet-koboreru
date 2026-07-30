#define NANOPRINTF_IMPLEMENTATION

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0

#include <debug.h>

#include <drivers/uart.h>

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = npf_vpprintf(&uart_putc, NULL, fmt, args);
    va_end(args);
    return ret;
}

void hexdump(const void *data, uint32_t size)
{
    const uint8_t *ptr = (const uint8_t *)data;

    for (uint32_t i = 0; i < size; i += 16) {
        printf("%08lx: ", (unsigned long)(ptr + i));

        for (uint32_t j = 0; j < 16; j++) {
            if (i + j < size)
                printf("%02x ", ptr[i + j]);
            else
                printf("   ");
            if (j == 7)
                printf(" ");
        }

        printf(" |");
        for (uint32_t j = 0; j < 16 && i + j < size; j++) {
            uint8_t c = ptr[i + j];
            printf("%c", (c >= 32 && c < 127) ? c : '.');
        }
        printf("|\n");
    }
}
