#include <libc/memory.h>

int memcmp(const void *a, const void *b, unsigned int n) {
    const uint8_t *p = a;
    const uint8_t *q = b;
    while (n--) {
        if (*p != *q)
            return *p - *q;
        p++;
        q++;
    }
    return 0;
}

void *memcpy(void *dst, const void *src, unsigned int n) {
    char *d = dst;
    const char *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

void *memset(void *dst, int c, unsigned int n) {
    uint8_t *p = dst;
    while (n--)
        *p++ = (uint8_t)c;
    return dst;
}