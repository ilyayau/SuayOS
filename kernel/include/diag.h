#pragma once
#include <stdint.h>

void klog(const char *msg);

__attribute__((noreturn)) void panic(const char *msg);
__attribute__((noreturn)) void panic_at(const char *file, int line, const char *msg);
__attribute__((noreturn)) void panic_assert(const char *file, int line, const char *expr);
__attribute__((noreturn)) void panic_assert_msg(const char *file, int line, const char *expr, const char *msg);

#define ASSERT(x) ((x) ? (void)0 : panic_assert(__FILE__, __LINE__, #x))
#define ASSERT_MSG(x, msg) ((x) ? (void)0 : panic_assert_msg(__FILE__, __LINE__, #x, (msg)))

#define PANIC(msg) panic_at(__FILE__, __LINE__, (msg))

void print_dec(uint64_t n, char *buf);
void print_hex(uint64_t n, char *buf, int prefix);
