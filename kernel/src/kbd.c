#include "../include/kbd.h"
#include "../include/io.h"
#include "../include/vga.h"
#include "../include/serial.h"
#include "../include/pic.h"
#include <stdint.h>

// Ring buffer for keyboard input
static char kbd_buf[KBD_BUF_SIZE];
static volatile int kbd_head = 0, kbd_tail = 0;

// Minimal US scancode to ASCII (no shift, no extended)
static const char scancode_ascii[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b', /* 0x0E: Backspace */
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', /* 0x1C: Enter */
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`', 0,  '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*',0, ' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void kbd_init(void) {
    // Nothing needed for now; handler is installed in IDT setup
}

// Called from IRQ1 handler
void kbd_handle_irq(void) {
    uint8_t sc = inb(0x60);
    if (sc & 0x80) return; // Ignore key releases
    char c = scancode_ascii[sc];
    if (!c) return;
    // Echo to VGA and serial
    vga_putc(c);
    serial_putc(c);
    // Push to ring buffer
    int next = (kbd_head + 1) % KBD_BUF_SIZE;
    if (next != kbd_tail) {
        kbd_buf[kbd_head] = c;
        kbd_head = next;
    }
}

int kbd_read_char(void) {
    if (kbd_head == kbd_tail) return -1;
    char c = kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUF_SIZE;
    return (unsigned char)c;
}
