#pragma once
#include <stdint.h>
void vmm_init(uint64_t kernel_phys_base, uint64_t kernel_phys_end);
void vmm_map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags);
#define KERNEL_VMA_BASE 0xFFFFFFFF80000000ULL
#define VMM_PRESENT 0x001
#define VMM_RW      0x002
#define VMM_USER    0x004
