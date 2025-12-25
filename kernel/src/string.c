#include "../include/string.h"
typedef unsigned long size_t;
int strcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) ++a, ++b;
    return (unsigned char)*a - (unsigned char)*b;
}
size_t strlen(const char *s) {
    size_t n = 0;
    while (s[n]) ++n;
    return n;
}

void *memcpy(void *dst, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < n; ++i) d[i] = s[i];
    return dst;
}

void *memset(void *dst, int c, size_t n) {
    unsigned char *d = (unsigned char *)dst;
    unsigned char v = (unsigned char)c;
    for (size_t i = 0; i < n; ++i) d[i] = v;
    return dst;
}
