#include <libc/string.h>

unsigned int strlen(const char *s) {
    const char *p = s;
    while (*p)
        p++;
    return (unsigned int)(p - s);
}

int streq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a++ != *b++)
            return 0;
    }
    return *a == *b;
}

void strnarrow(const uint16_t *src, char *dst, int max) {
    int i;
    for (i = 0; i < max && src[i]; i++)
        dst[i] = (char)(src[i] & 0x7F);
    dst[i] = '\0';
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *(const unsigned char *)a - *(const unsigned char *)b;
}

int strncmp(const char *a, const char *b, unsigned int n) {
    while (n && *a && *a == *b) {
        a++;
        b++;
        n--;
    }
    if (n == 0)
        return 0;
    return *(const unsigned char *)a - *(const unsigned char *)b;
}