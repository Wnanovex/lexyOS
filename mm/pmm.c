#include <mm/pmm.h>
#include <ui/console.h>

// Bitmap to track page usage (1 bit per page)
// Each uint32_t tracks 32 pages
#define MAX_BLOCKS 32768  // Support up to 4GB RAM (32768 * 32 * 4KB)
static uint32_t memory_bitmap[MAX_BLOCKS];
static uint64_t total_blocks = 0;
static uint64_t used_blocks = 0;
static uint64_t total_memory = 0;

// Helper: Set bit in bitmap (mark page as used)
static inline void bitmap_set(uint32_t bit) {
    uint32_t block = bit / 32;
    uint32_t offset = bit % 32;
    if (block < MAX_BLOCKS) {
        memory_bitmap[block] |= (1 << offset);
    }
}

// Helper: Clear bit in bitmap (mark page as free)
static inline void bitmap_clear(uint32_t bit) {
    uint32_t block = bit / 32;
    uint32_t offset = bit % 32;
    if (block < MAX_BLOCKS) {
        memory_bitmap[block] &= ~(1 << offset);
    }
}

// Helper: Test bit in bitmap
static inline int bitmap_test(uint32_t bit) {
    uint32_t block = bit / 32;
    uint32_t offset = bit % 32;
    if (block < MAX_BLOCKS) {
        return (memory_bitmap[block] & (1 << offset)) != 0;
    }
    return 0;
}

// Helper: Find first free page
static int find_free_page(void) {
    for (uint32_t i = 0; i < total_blocks; i++) {
        if (memory_bitmap[i] != 0xFFFFFFFF) {
            // This block has free pages
            for (int j = 0; j < 32; j++) {
                if (!(memory_bitmap[i] & (1 << j))) {
                    return i * 32 + j;
                }
            }
        }
    }
    return -1;  // Out of memory
}

// Helper: Find contiguous free pages
static int find_free_pages(size_t count) {
    if (count == 0) return -1;
    
    uint32_t found = 0;
    int start = -1;
    
    for (uint32_t page = 0; page < total_blocks * 32; page++) {
        if (!bitmap_test(page)) {
            if (start == -1) {
                start = page;
            }
            found++;
            
            if (found == count) {
                return start;
            }
        } else {
            start = -1;
            found = 0;
        }
    }
    
    return -1;  // Not enough contiguous pages
}

void pmm_init(uint64_t mem_size) {
    total_memory = mem_size;
    total_blocks = (mem_size / PAGE_SIZE) / 32;
    
    if (total_blocks > MAX_BLOCKS) {
        total_blocks = MAX_BLOCKS;
    }
    
    // Initially mark all memory as used
    for (uint32_t i = 0; i < MAX_BLOCKS; i++) {
        memory_bitmap[i] = 0xFFFFFFFF;
    }
    
    // Mark available memory as free (skip first 16MB for kernel)
    uint64_t kernel_pages = (16 * 1024 * 1024) / PAGE_SIZE;
    uint64_t total_pages = mem_size / PAGE_SIZE;
    
    for (uint64_t page = kernel_pages; page < total_pages; page++) {
        bitmap_clear(page);
    }
    
    used_blocks = kernel_pages / 32;
    
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("[PMM] Physical Memory Manager initialized\n");
    console_write("[PMM] Total Memory: ");
    
    // Print memory size in MB
    uint64_t mem_mb = mem_size / (1024 * 1024);
    if (mem_mb >= 1000) {
        console_putchar('0' + (mem_mb / 1000));
        mem_mb %= 1000;
    }
    if (mem_mb >= 100) console_putchar('0' + (mem_mb / 100));
    if (mem_mb >= 10) console_putchar('0' + ((mem_mb / 10) % 10));
    console_putchar('0' + (mem_mb % 10));
    console_write(" MB\n");
    
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
}

uint64_t pmm_alloc_page(void) {
    int page = find_free_page();
    
    if (page == -1) {
        return 0;  // Out of memory
    }
    
    bitmap_set(page);
    used_blocks++;
    
    return (uint64_t)page * PAGE_SIZE;
}

void pmm_free_page(uint64_t page_addr) {
    if (page_addr == 0) return;
    
    uint32_t page = page_addr / PAGE_SIZE;
    
    if (bitmap_test(page)) {
        bitmap_clear(page);
        used_blocks--;
    }
}

uint64_t pmm_alloc_pages(size_t count) {
    if (count == 0) return 0;
    
    int start_page = find_free_pages(count);
    
    if (start_page == -1) {
        return 0;  // Not enough contiguous pages
    }
    
    // Mark all pages as used
    for (size_t i = 0; i < count; i++) {
        bitmap_set(start_page + i);
        used_blocks++;
    }
    
    return (uint64_t)start_page * PAGE_SIZE;
}

void pmm_free_pages(uint64_t page_addr, size_t count) {
    if (page_addr == 0 || count == 0) return;
    
    uint32_t start_page = page_addr / PAGE_SIZE;
    
    for (size_t i = 0; i < count; i++) {
        if (bitmap_test(start_page + i)) {
            bitmap_clear(start_page + i);
            used_blocks--;
        }
    }
}

uint64_t pmm_get_total_memory(void) {
    return total_memory;
}

uint64_t pmm_get_used_memory(void) {
    return used_blocks * 32 * PAGE_SIZE;
}

uint64_t pmm_get_free_memory(void) {
    return total_memory - pmm_get_used_memory();
}
