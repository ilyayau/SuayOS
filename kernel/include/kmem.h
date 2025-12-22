#pragma once
#include <stddef.h>
void kmem_init(void);
void *kmalloc(size_t size, size_t align);
void kfree(void *ptr);
