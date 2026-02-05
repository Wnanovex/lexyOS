#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

// Initialize kernel heap
void heap_init(void* start, size_t size);

// Allocate memory
void* kmalloc(size_t size);
void* kmalloc_aligned(size_t size, size_t alignment);
void* kcalloc(size_t nmemb, size_t size);
void* krealloc(void* ptr, size_t size);

// Free memory
void kfree(void* ptr);

// Get heap statistics
size_t heap_get_used(void);
size_t heap_get_free(void);


void heap_debug_info(void);

#endif // HEAP_H
