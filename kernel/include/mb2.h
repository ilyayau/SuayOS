#pragma once
#include <stdint.h>
struct mb2_tag {
    uint32_t type;
    uint32_t size;
};
struct mb2_memmap_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};
#define MB2_TAG_TYPE_END 0
#define MB2_TAG_TYPE_MMAP 6
#define MB2_MMAP_TYPE_USABLE 1
void mb2_parse(uint64_t mb_info);
uint64_t mb2_get_usable_base(void);
uint64_t mb2_get_usable_size(void);
