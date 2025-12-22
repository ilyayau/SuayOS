#include "../../include/irq.h"
#include "../../include/serial.h"
#include "../../include/pic.h"
#include "../../include/kbd.h"
#include <stdint.h>
static volatile uint64_t tick = 0;
uint64_t get_ticks(void) { return tick; }
static void u64_to_str(uint64_t n, char *buf) {
    char tmp[21];
    int i = 0, j = 0;
    if (n == 0) { buf[0] = '0'; buf[1] = 0; return; }
    while (n) { tmp[i++] = '0' + (n % 10); n /= 10; }
    while (i--) buf[j++] = tmp[i];
    buf[j] = 0;
}
void irq0_handler(void) {
    ++tick;
    if (tick % 100 == 0) {
        char msg[32] = "tick ";
        u64_to_str(tick / 100, msg + 5);
        msg[5 + (tick / 100 == 0 ? 1 : 0) + 20] = '\n';
        serial_write(msg);
    }
    pic_send_eoi(0);
}
void irq1_handler(void) {
    kbd_handle_irq();
    pic_send_eoi(1);
}
