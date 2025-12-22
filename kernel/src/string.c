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
