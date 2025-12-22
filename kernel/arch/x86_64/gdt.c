

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

static struct gdt_entry gdt[7];
static struct gdt_ptr gdtp;
static struct tss tss __attribute__((aligned(16)));

void tss_load(void) {
    __asm__ volatile ("ltr %%ax" : : "a"(0x30));
}

void gdt_init(void) {
    // Null
    gdt[0] = (struct gdt_entry){0};
    // Kernel code
    gdt[1] = (struct gdt_entry){
        .limit_low = 0xFFFF, .base_low = 0, .base_mid = 0, .access = 0x9A, .gran = 0xA0, .base_high = 0
    };
    // Kernel data
    gdt[2] = (struct gdt_entry){
        .limit_low = 0xFFFF, .base_low = 0, .base_mid = 0, .access = 0x92, .gran = 0xA0, .base_high = 0
    };
    // User code
    gdt[3] = (struct gdt_entry){
        .limit_low = 0xFFFF, .base_low = 0, .base_mid = 0, .access = 0xFA, .gran = 0xA0, .base_high = 0
    };
    // User data
    gdt[4] = (struct gdt_entry){
        .limit_low = 0xFFFF, .base_low = 0, .base_mid = 0, .access = 0xF2, .gran = 0xA0, .base_high = 0
    };
    // TSS (64-bit)
    uint64_t tss_base = (uint64_t)&tss;
    gdt[5] = (struct gdt_entry){
        .limit_low = sizeof(struct tss)-1,
        .base_low = tss_base & 0xFFFF,
        .base_mid = (tss_base >> 16) & 0xFF,
        .access = 0x89, // present, type=9 (64-bit TSS)
        .gran = ((tss_base >> 24) & 0xFF),
        .base_high = (tss_base >> 32) & 0xFF
    };
    gdt[6] = (struct gdt_entry){0}; // TSS high (not used in this simple GDT)

    gdtp.limit = sizeof(gdt)-1;
    gdtp.base = (uint64_t)&gdt;
    __asm__ volatile ("lgdt %0" : : "m"(gdtp));
    tss_load();
}
