#include "../../include/pit.h"
#include "../../include/io.h"
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43
void pit_init(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}
