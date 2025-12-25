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

struct mb2_tag_module {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char cmdline[0];
};
#define MB2_TAG_TYPE_END 0
#define MB2_TAG_TYPE_MODULE 3
#define MB2_TAG_TYPE_MMAP 6
#define MB2_MMAP_TYPE_USABLE 1
void mb2_parse(uint64_t mb_info);
uint64_t mb2_get_usable_base(void);
uint64_t mb2_get_usable_size(void);

uint64_t mb2_get_module_start(void);
uint64_t mb2_get_module_end(void);
const char *mb2_get_module_cmdline(void);
