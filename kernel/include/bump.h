#pragma once
#include <stdint.h>
void bump_init(uint64_t base, uint64_t size);
void *bump_alloc(uint64_t size, uint64_t align);
