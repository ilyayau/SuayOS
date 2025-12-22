#include "../include/vmm.h"
#include "../include/pmm.h"
#include "../include/diag.h"
#include <stdint.h>
#define PAGE_SIZE 4096
#define ENTRIES 512
static uint64_t *pml4 = 0;
static uint64_t phys_to_virt(uint64_t p) { return p + KERNEL_VMA_BASE; }
static uint64_t virt_to_phys(uint64_t v) { return v - KERNEL_VMA_BASE; }
void vmm_map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx   = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx   = (vaddr >> 12) & 0x1FF;
    if (!pml4) panic("vmm_map_page: pml4 not set");
    uint64_t *pdpt = (uint64_t *)phys_to_virt(pml4[pml4_idx] & ~0xFFFULL);
    if (!(pml4[pml4_idx] & VMM_PRESENT)) {
        pdpt = pmm_alloc_pages(1);
        pml4[pml4_idx] = virt_to_phys((uint64_t)pdpt) | VMM_PRESENT | VMM_RW;
        pdpt = (uint64_t *)phys_to_virt((uint64_t)pdpt);
        for (int i = 0; i < ENTRIES; ++i) pdpt[i] = 0;
    }
    uint64_t *pd = (uint64_t *)phys_to_virt(pdpt[pdpt_idx] & ~0xFFFULL);
    if (!(pdpt[pdpt_idx] & VMM_PRESENT)) {
        pd = pmm_alloc_pages(1);
        pdpt[pdpt_idx] = virt_to_phys((uint64_t)pd) | VMM_PRESENT | VMM_RW;
        pd = (uint64_t *)phys_to_virt((uint64_t)pd);
        for (int i = 0; i < ENTRIES; ++i) pd[i] = 0;
    }
    uint64_t *pt = (uint64_t *)phys_to_virt(pd[pd_idx] & ~0xFFFULL);
    if (!(pd[pd_idx] & VMM_PRESENT)) {
        pt = pmm_alloc_pages(1);
        pd[pd_idx] = virt_to_phys((uint64_t)pt) | VMM_PRESENT | VMM_RW;
        pt = (uint64_t *)phys_to_virt((uint64_t)pt);
        for (int i = 0; i < ENTRIES; ++i) pt[i] = 0;
    }
    pt[pt_idx] = paddr | (flags & 0xFFF) | VMM_PRESENT;
}
void vmm_init(uint64_t kernel_phys_base, uint64_t kernel_phys_end) {
    pml4 = pmm_alloc_pages(1);
    for (int i = 0; i < ENTRIES; ++i) pml4[i] = 0;
    // Identity-map first 2MB for boot
    for (uint64_t addr = 0; addr < 0x200000; addr += PAGE_SIZE)
        vmm_map_page(addr, addr, VMM_RW);
    // Map kernel to higher half
    uint64_t ksize = kernel_phys_end - kernel_phys_base;
    for (uint64_t off = 0; off < ksize; off += PAGE_SIZE)
        vmm_map_page(KERNEL_VMA_BASE + off, kernel_phys_base + off, VMM_RW);
    // Load new PML4
    __asm__ volatile ("mov %0, %%cr3" : : "r"(virt_to_phys((uint64_t)pml4)) : "memory");
}
