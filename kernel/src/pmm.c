
#include "../include/pmm.h"
#include "../include/mb2.h"
#include "../include/diag.h"
#include "../include/serial.h"
#include <stdint.h>
#define PAGE_SIZE 4096
#define MAX_PAGES 262144 // 1GiB max



// ...existing code...


// ...existing code...

#include <stdint.h>
#define PAGE_SIZE 4096
#define MAX_PAGES 262144 // 1GiB max
static uint8_t bitmap[MAX_PAGES/8];
static uint64_t total_pages = 0, usable_pages = 0, reserved_pages = 0, base_addr = 0;
static inline void set_bit(uint64_t i) { bitmap[i/8] |= (1 << (i%8)); }
static inline void clear_bit(uint64_t i) { bitmap[i/8] &= ~(1 << (i%8)); }
static inline int test_bit(uint64_t i) { return (bitmap[i/8] >> (i%8)) & 1; }
void pmm_init(uint64_t mb_info) {
    for (uint64_t i = 0; i < MAX_PAGES/8; ++i) bitmap[i] = 0xFF;
    const struct mb2_tag *tag = (const struct mb2_tag *)(mb_info + 8);
    while (tag->type != MB2_TAG_TYPE_END) {
        if (tag->type == MB2_TAG_TYPE_MMAP) {
            uint32_t entry_size = *(uint32_t *)((const char *)tag + 8);
            const char *entry_ptr = (const char *)tag + 16;
            uint32_t entries = (tag->size - 16) / entry_size;
            for (uint32_t i = 0; i < entries; ++i) {
                const struct mb2_memmap_entry *entry = (const struct mb2_memmap_entry *)(entry_ptr + i * entry_size);
                uint64_t first = entry->base / PAGE_SIZE;
                uint64_t last = (entry->base + entry->length) / PAGE_SIZE;
                if (entry->type == MB2_MMAP_TYPE_USABLE) {
                    for (uint64_t p = first; p < last; ++p) clear_bit(p);
                    usable_pages += last - first;
                    if (base_addr == 0) base_addr = entry->base;
                } else {
                    reserved_pages += last - first;
                }
                if (last > total_pages) total_pages = last;
            }
        }
        tag = (const struct mb2_tag *)((const char *)tag + ((tag->size + 7) & ~7));
    }
    char buf[64];
    serial_write("PMM: total "); print_dec(total_pages, buf); serial_write(buf);
    serial_write(", usable "); print_dec(usable_pages, buf); serial_write(buf);
    serial_write(", reserved "); print_dec(reserved_pages, buf); serial_write(buf);
    serial_write(" pages\n");
}
void *pmm_alloc_pages(uint64_t n) {
    uint64_t found = 0, start = 0;
    for (uint64_t i = 0; i < total_pages; ++i) {
        if (!test_bit(i)) {
            if (found == 0) start = i;
            if (++found == n) {
                for (uint64_t j = 0; j < n; ++j) set_bit(start + j);
                return (void *)(base_addr + start * PAGE_SIZE);
            }
        } else {
            found = 0;
        }
    }
    panic("pmm_alloc_pages OOM");
}

void pmm_free_pages(void *ptr, uint64_t n) {
    uint64_t idx = ((uint64_t)ptr - base_addr) / PAGE_SIZE;
    for (uint64_t j = 0; j < n; ++j) {
        ASSERT(test_bit(idx + j));
        clear_bit(idx + j);
    }
}

uint64_t pmm_total_pages(void) { return total_pages; }
uint64_t pmm_free_pages_count(void) {
    uint64_t free = 0;
    for (uint64_t i = 0; i < total_pages; ++i)
        if (!test_bit(i)) ++free;
    return free;
}
