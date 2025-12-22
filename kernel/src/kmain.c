
#include "../include/vga.h"

void kmain(void) {
    vga_clear();
    vga_print("SuayOS booted");
    for (;;) {}
}
