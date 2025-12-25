#include "../include/kmem.h"
#include "../include/pmm.h"
#include "../include/vmm.h"
#include "../include/diag.h"
#include <stdint.h>
#define HEAP_START 0xFFFFFFFF90000000ULL
#define HEAP_INITIAL_PAGES 16
#define PAGE_SIZE 4096
struct block {
    struct block *next;
    size_t size;
    int free;
    uint32_t _pad;
    uint64_t _reserved;
};
static struct block *heap_head = 0;
static uint64_t heap_end = HEAP_START;
static void heap_expand(size_t min_size) {
    size_t to_alloc = ((min_size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    for (size_t off = 0; off < to_alloc; off += PAGE_SIZE) {
        void *page = pmm_alloc_pages(1);
        vmm_map_page(heap_end + off, (uint64_t)page, VMM_RW);
    }
    heap_end += to_alloc;
}
void kmem_init(void) {
    heap_head = (struct block *)HEAP_START;
    heap_expand(HEAP_INITIAL_PAGES * PAGE_SIZE);
    heap_head->next = 0;
    heap_head->size = HEAP_INITIAL_PAGES * PAGE_SIZE - sizeof(struct block);
    heap_head->free = 1;
}
static struct block *find_block(size_t size, size_t align) {
    (void)align;
    struct block *cur = heap_head;
    while (cur) {
        if (cur->free && cur->size >= size) {
            return cur;
        }
        cur = cur->next;
    }
    return 0;
}
void *kmalloc(size_t size, size_t align) {
    if (!size) return 0;

    if (align == 0) align = 16;
    if ((align & (align - 1)) != 0 || align > 16) panic("kmalloc bad align");
    size = (size + align - 1) & ~(align - 1);

    struct block *b = find_block(size, align);
    if (!b) {
        heap_expand(size + align + sizeof(struct block));
        b = find_block(size, align);
        if (!b) panic("kmalloc OOM");
    }

    // Block payload starts at (b + 1), which is 16-byte aligned because:
    // - HEAP_START is 16-byte aligned
    // - sizeof(struct block) is 32 bytes
    // - we allocate sizes rounded up to 16
    uintptr_t payload = (uintptr_t)(b + 1);
    if ((payload & (align - 1)) != 0) panic("kmalloc misaligned header");

    if (b->size > size + sizeof(struct block)) {
        struct block *split = (struct block *)(payload + size);
        split->next = b->next;
        split->size = b->size - size - sizeof(struct block);
        split->free = 1;
        b->next = split;
        b->size = size;
    }
    b->free = 0;
    return (void *)payload;
}
void kfree(void *ptr) {
    if (!ptr) return;
    struct block *b = (struct block *)ptr - 1;
    b->free = 1;
    // Coalesce
    struct block *cur = heap_head;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size += sizeof(struct block) + cur->next->size;
            cur->next = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
}
