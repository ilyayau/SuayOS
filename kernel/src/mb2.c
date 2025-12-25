#include "../include/mb2.h"
#include "../include/serial.h"
#include "../include/diag.h"
static uint64_t usable_base = 0, usable_size = 0;
static uint64_t module_start = 0, module_end = 0;
static const char *module_cmdline = 0;
void mb2_parse(uint64_t mb_info) {
    const struct mb2_tag *tag = (const struct mb2_tag *)(mb_info + 8);
    while (tag->type != MB2_TAG_TYPE_END) {
        if (tag->type == MB2_TAG_TYPE_MODULE && module_start == 0) {
            const struct mb2_tag_module *m = (const struct mb2_tag_module *)tag;
            module_start = m->mod_start;
            module_end = m->mod_end;
            module_cmdline = m->cmdline;
        }
        if (tag->type == MB2_TAG_TYPE_MMAP) {
            uint32_t entry_size = *(uint32_t *)((const char *)tag + 8);
            uint32_t entry_version = *(uint32_t *)((const char *)tag + 12);
            (void)entry_version;
            const char *entry_ptr = (const char *)tag + 16;
            uint32_t entries = (tag->size - 16) / entry_size;
            for (uint32_t i = 0; i < entries; ++i) {
                const struct mb2_memmap_entry *entry = (const struct mb2_memmap_entry *)(entry_ptr + i * entry_size);
                if (entry->type == MB2_MMAP_TYPE_USABLE && entry->length > usable_size) {
                    usable_base = entry->base;
                    usable_size = entry->length;
                }
            }
        }
        tag = (const struct mb2_tag *)((const char *)tag + ((tag->size + 7) & ~7));
    }
    char msg[64] = "RAM ";
    uint64_t kb = usable_size / 1024;
    int i = 4;
    if (kb == 0) { msg[i++] = '0'; }
    else {
        char tmp[20]; int j = 0;
        while (kb) { tmp[j++] = '0' + (kb % 10); kb /= 10; }
        while (j--) msg[i++] = tmp[j];
    }
    msg[i++] = 'K'; msg[i++] = 'B'; msg[i++] = '\n'; msg[i] = 0;
    serial_write(msg);

    if (module_start && module_end && module_end > module_start) {
        char hex[32];
        serial_write("module: ");
        print_hex(module_start, hex, 1); serial_write(hex);
        serial_write("..");
        print_hex(module_end, hex, 1); serial_write(hex);
        if (module_cmdline && module_cmdline[0]) {
            serial_write(" ");
            serial_write(module_cmdline);
        }
        serial_write("\n");
    }
}
uint64_t mb2_get_usable_base(void) { return usable_base; }
uint64_t mb2_get_usable_size(void) { return usable_size; }

uint64_t mb2_get_module_start(void) { return module_start; }
uint64_t mb2_get_module_end(void) { return module_end; }
const char *mb2_get_module_cmdline(void) { return module_cmdline; }
