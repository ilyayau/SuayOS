#include "../../include/gdt.h"
#include <stdint.h>
struct __attribute__((packed)) gdt_ptr {
    uint16_t limit;
    uint64_t base;
};
static struct gdt_ptr gdtp = {0};
void gdt_init(void) {
    __asm__ volatile ("lgdt %0" : : "m"(gdtp));
}
