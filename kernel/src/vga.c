#include "../include/vga.h"
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_ADDR ((volatile uint16_t*)0xB8000)
static uint16_t vga_entry(char c, uint8_t color) { return (uint16_t)c | ((uint16_t)color << 8); }
void vga_clear(void) {
    for (uint16_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i)
        VGA_ADDR[i] = vga_entry(' ', 0x07);
}
void vga_print(const char *s) {
    volatile uint16_t *vga = VGA_ADDR;
    uint16_t i = 0;
    while (s[i] && i < VGA_WIDTH) {
        vga[i] = vga_entry(s[i], 0x07);
        ++i;
    }
}
