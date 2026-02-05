#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define PAGES_PER_BLOCK 32  // 32 pages per bitmap block (32 bits)

// Initialize physical memory manager
void pmm_init(uint64_t mem_size);

// Allocate/free physical pages
uint64_t pmm_alloc_page(void);
void pmm_free_page(uint64_t page_addr);

// Allocate/free multiple contiguous pages
uint64_t pmm_alloc_pages(size_t count);
void pmm_free_pages(uint64_t page_addr, size_t count);

// Get memory statistics
uint64_t pmm_get_total_memory(void);
uint64_t pmm_get_used_memory(void);
uint64_t pmm_get_free_memory(void);

#endif // PMM_H
