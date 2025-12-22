#pragma once
#include <stdint.h>
void panic(const char *msg);
#define ASSERT(x) ((x) ? (void)0 : panic_assert(__FILE__, __LINE__, #x))
void panic_assert(const char *file, int line, const char *expr);
void print_dec(uint64_t n, char *buf);
void print_hex(uint64_t n, char *buf, int prefix);
