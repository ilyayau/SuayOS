

#include "../../include/gdt.h"
#include <stdint.h>

struct __attribute__((packed)) gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t gran;
    uint8_t base_high;
};

struct __attribute__((packed)) gdt_ptr {
    uint16_t limit;
    uint64_t base;
};

struct __attribute__((packed)) tss {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1, ist2, ist3, ist4, ist5, ist6, ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
};

static struct gdt_entry gdt[7] __attribute__((aligned(16)));
static struct gdt_ptr gdtp;
static struct tss tss __attribute__((aligned(16)));

void tss_load(void) {
    __asm__ volatile ("ltr %%ax" : : "a"(0x30));
}

void gdt_init(void) {
    // Null
    *(uint64_t *)&gdt[0] = 0;
    // Kernel code
    *(uint64_t *)&gdt[1] = 0x00af9a000000ffffULL;
    // Kernel data
    *(uint64_t *)&gdt[2] = 0x00cf92000000ffffULL;
    // User code
    *(uint64_t *)&gdt[3] = 0x00affa000000ffffULL;
    // User data
    *(uint64_t *)&gdt[4] = 0x00cff2000000ffffULL;
    // TSS (64-bit)
    uint64_t tss_base = (uint64_t)&tss;
    *(uint64_t *)&gdt[5] =
        ((sizeof(struct tss)-1) & 0xFFFF) |
        ((tss_base & 0xFFFF) << 16) |
        (((tss_base >> 16) & 0xFF) << 32) |
        ((0x89ULL) << 40) |
        ((((tss_base >> 24) & 0xFF)) << 48) |
        (((tss_base >> 32) & 0xFF) << 56);
    *(uint64_t *)&gdt[6] = 0;

    gdtp.limit = sizeof(gdt)-1;
    gdtp.base = (uint64_t)&gdt;
    __asm__ volatile ("lgdt %0" : : "m"(gdtp));
    tss_load();
}
