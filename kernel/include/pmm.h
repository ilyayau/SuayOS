#pragma once
#include <stdint.h>
void pmm_init(uint64_t mb_info);
void *pmm_alloc_pages(uint64_t n);
void pmm_free_pages(void *ptr, uint64_t n);
