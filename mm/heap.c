#include <mm/heap.h>
#include <ui/console.h>

// Memory block header
typedef struct block_header {
    size_t size;                    // Size of the block (excluding header)
    struct block_header* next;      // Next block in free list
    int is_free;                    // 1 if free, 0 if allocated
} block_header_t;

#define BLOCK_HEADER_SIZE sizeof(block_header_t)
#define ALIGN_SIZE 16

static uint8_t* heap_start = NULL;
static uint8_t* heap_end = NULL;
static block_header_t* free_list = NULL;
static size_t total_size = 0;
static size_t used_size = 0;

// Helper: Align size to ALIGN_SIZE
static inline size_t align_size(size_t size) {
    return (size + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
}

// Helper: Find free block that fits
static block_header_t* find_free_block(size_t size) {
    block_header_t* current = free_list;
    
    while (current) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Helper: Split block if it's too large
static void split_block(block_header_t* block, size_t size) {
    if (block->size >= size + BLOCK_HEADER_SIZE + ALIGN_SIZE) {
        // Create new block from remaining space
        block_header_t* new_block = (block_header_t*)((uint8_t*)block + BLOCK_HEADER_SIZE + size);
        new_block->size = block->size - size - BLOCK_HEADER_SIZE;
        new_block->is_free = 1;
        new_block->next = block->next;
        
        block->size = size;
        block->next = new_block;
    }
}

// Helper: Merge adjacent free blocks
static void merge_free_blocks(void) {
    block_header_t* current = free_list;
    
    while (current && current->next) {
        if (current->is_free && current->next->is_free) {
            // Merge with next block
            current->size += BLOCK_HEADER_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void heap_init(void* start, size_t size) {
    heap_start = (uint8_t*)start;
    heap_end = heap_start + size;
    total_size = size;
    used_size = 0;
    
    // Create initial free block
    free_list = (block_header_t*)heap_start;
    free_list->size = size - BLOCK_HEADER_SIZE;
    free_list->next = NULL;
    free_list->is_free = 1;
    
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("[HEAP] Kernel heap initialized (");
    
    // Print heap size in KB/MB
    if (size >= 1024 * 1024) {
        size_t mb = size / (1024 * 1024);
        console_putchar('0' + (mb / 10));
        console_putchar('0' + (mb % 10));
        console_write(" MB)\n");
    } else {
        size_t kb = size / 1024;
        if (kb >= 1000) console_putchar('0' + (kb / 1000));
        if (kb >= 100) console_putchar('0' + ((kb / 100) % 10));
        if (kb >= 10) console_putchar('0' + ((kb / 10) % 10));
        console_putchar('0' + (kb % 10));
        console_write(" KB)\n");
    }
    
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
}

void* kmalloc(size_t size) {
    if (size == 0 || !heap_start) {
        return NULL;
    }
    
    // Align size
    size = align_size(size);
    
    // Find free block
    block_header_t* block = find_free_block(size);
    
    if (!block) {
        return NULL;  // Out of memory
    }
    
    // Split block if possible
    split_block(block, size);
    
    // Mark as allocated
    block->is_free = 0;
    used_size += size + BLOCK_HEADER_SIZE;
    
    // Return pointer to data (after header)
    return (void*)((uint8_t*)block + BLOCK_HEADER_SIZE);
}

void* kmalloc_aligned(size_t size, size_t alignment) {
    // Simple implementation: just align to alignment
    size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
    return kmalloc(aligned_size);
}

void* kcalloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void* ptr = kmalloc(total);
    
    if (ptr) {
        // Zero out memory
        uint8_t* p = (uint8_t*)ptr;
        for (size_t i = 0; i < total; i++) {
            p[i] = 0;
        }
    }
    
    return ptr;
}

void* krealloc(void* ptr, size_t size) {
    if (!ptr) {
        return kmalloc(size);
    }
    
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    // Get old block header
    block_header_t* old_block = (block_header_t*)((uint8_t*)ptr - BLOCK_HEADER_SIZE);
    
    if (old_block->size >= size) {
        // Block is large enough, just return it
        return ptr;
    }
    
    // Allocate new block
    void* new_ptr = kmalloc(size);
    if (!new_ptr) {
        return NULL;
    }
    
    // Copy old data
    uint8_t* src = (uint8_t*)ptr;
    uint8_t* dst = (uint8_t*)new_ptr;
    for (size_t i = 0; i < old_block->size; i++) {
        dst[i] = src[i];
    }
    
    // Free old block
    kfree(ptr);
    
    return new_ptr;
}

void kfree(void* ptr) {
    if (!ptr || !heap_start) {
        return;
    }
    
    // Get block header
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - BLOCK_HEADER_SIZE);
    
    // Mark as free
    block->is_free = 1;
    used_size -= block->size + BLOCK_HEADER_SIZE;
    
    // Merge adjacent free blocks
    merge_free_blocks();
}

size_t heap_get_used(void) {
    return used_size;
}

size_t heap_get_free(void) {
    return total_size - used_size;
}
