#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>

// Page table flags
#define PAGE_PRESENT    (1 << 0)
#define PAGE_WRITABLE   (1 << 1)
#define PAGE_USER       (1 << 2)
#define PAGE_WRITETHROUGH (1 << 3)
#define PAGE_CACHE_DISABLE (1 << 4)

// Page directory/table structure
typedef struct {
    uint64_t entries[512];
} __attribute__((aligned(4096))) page_table_t;

// Initialize virtual memory manager
void vmm_init(void);

// Map virtual address to physical address
int vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);

// Unmap virtual address
void vmm_unmap_page(uint64_t virt_addr);

// Get physical address from virtual address
uint64_t vmm_get_physical_address(uint64_t virt_addr);

// Switch page directory
void vmm_switch_page_directory(page_table_t* pml4);

#endif // VMM_H
