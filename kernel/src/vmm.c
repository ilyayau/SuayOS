#include "../include/vmm.h"
#include "../include/pmm.h"
#include "../include/diag.h"
#include <stdint.h>
#define PAGE_SIZE 4096
#define ENTRIES 512
static uint64_t *pml4 = 0;
static uint64_t pml4_phys = 0;
static uint64_t phys_to_virt(uint64_t p) { return p + KERNEL_VMA_BASE; }

void vmm_map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx   = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx   = (vaddr >> 12) & 0x1FF;
    if (!pml4) panic("vmm_map_page: pml4 not set");

    uint64_t pdpt_phys = pml4[pml4_idx] & ~0xFFFULL;
    uint64_t *pdpt = (uint64_t *)phys_to_virt(pdpt_phys);
    if (!(pml4[pml4_idx] & VMM_PRESENT)) {
        pdpt_phys = (uint64_t)pmm_alloc_pages(1);
        pml4[pml4_idx] = pdpt_phys | VMM_PRESENT | VMM_RW;
        pdpt = (uint64_t *)phys_to_virt(pdpt_phys);
        for (int i = 0; i < ENTRIES; ++i) pdpt[i] = 0;
    }

    uint64_t pd_phys = pdpt[pdpt_idx] & ~0xFFFULL;
    uint64_t *pd = (uint64_t *)phys_to_virt(pd_phys);
    if (!(pdpt[pdpt_idx] & VMM_PRESENT)) {
        pd_phys = (uint64_t)pmm_alloc_pages(1);
        pdpt[pdpt_idx] = pd_phys | VMM_PRESENT | VMM_RW;
        pd = (uint64_t *)phys_to_virt(pd_phys);
        for (int i = 0; i < ENTRIES; ++i) pd[i] = 0;
    }

    uint64_t pt_phys = pd[pd_idx] & ~0xFFFULL;
    uint64_t *pt = (uint64_t *)phys_to_virt(pt_phys);
    if (!(pd[pd_idx] & VMM_PRESENT)) {
        pt_phys = (uint64_t)pmm_alloc_pages(1);
        pd[pd_idx] = pt_phys | VMM_PRESENT | VMM_RW;
        pt = (uint64_t *)phys_to_virt(pt_phys);
        for (int i = 0; i < ENTRIES; ++i) pt[i] = 0;
    }

    pt[pt_idx] = (paddr & ~0xFFFULL) | (flags & 0xFFF) | VMM_PRESENT;
}
void vmm_init(uint64_t kernel_phys_base, uint64_t kernel_phys_end) {
    pml4_phys = (uint64_t)pmm_alloc_pages(1);
    pml4 = (uint64_t *)phys_to_virt(pml4_phys);
    for (int i = 0; i < ENTRIES; ++i) pml4[i] = 0;

    // Identity-map first 2MiB so execution continues safely after CR3 switch.
    for (uint64_t addr = 0; addr < 0x200000; addr += PAGE_SIZE)
        vmm_map_page(addr, addr, VMM_RW);

    // Establish a direct physmap: VA = KERNEL_VMA_BASE + phys.
    // Must cover at least early page-table allocations (often near 1MiB).
    uint64_t physmap_end = kernel_phys_end;
    if (physmap_end < 0x200000) physmap_end = 0x200000;
    for (uint64_t addr = 0; addr < physmap_end; addr += PAGE_SIZE)
        vmm_map_page(KERNEL_VMA_BASE + addr, addr, VMM_RW);

    // Load new PML4
    __asm__ volatile ("mov %0, %%cr3" : : "r"(pml4_phys) : "memory");
}
