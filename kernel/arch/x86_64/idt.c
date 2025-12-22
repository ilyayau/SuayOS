#include "../../include/idt.h"
#include "../../include/serial.h"
#include <stdint.h>
struct __attribute__((packed)) idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
};
struct __attribute__((packed)) idt_ptr {
    uint16_t limit;
    uint64_t base;
};
extern void isr_default_stub(void);
static struct idt_entry idt[256] = {0};
static struct idt_ptr idtp = {sizeof(idt)-1, (uint64_t)idt};
void idt_init(void) {
    uint64_t handler = (uint64_t)isr_default_stub;
    for (int i = 0; i < 256; ++i) {
        idt[i].offset_low = handler & 0xFFFF;
        idt[i].selector = 0x08;
        idt[i].ist = 0;
        idt[i].type_attr = 0x8E;
        idt[i].offset_mid = (handler >> 16) & 0xFFFF;
        idt[i].offset_high = (handler >> 32) & 0xFFFFFFFF;
        idt[i].zero = 0;
    }
    __asm__ volatile ("lidt %0" : : "m"(idtp));
}
