 #include "../include/bump.h"
#include "../include/serial.h"
#include "../include/diag.h"
static uint64_t bump_base = 0, bump_size = 0, bump_ptr = 0;
void bump_init(uint64_t base, uint64_t size) {
    bump_base = base;
    bump_size = size;
    bump_ptr = base;
}
void *bump_alloc(uint64_t size, uint64_t align) {
    uint64_t p = (bump_ptr + align - 1) & ~(align - 1);
    if (p + size > bump_base + bump_size)
        panic("bump_alloc OOM");
    bump_ptr = p + size;
    return (void *)p;
}
