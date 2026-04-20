#include "kernel/pmm.h"
#include <stddef.h>

// Simple bitmap-based physical memory manager implementation
namespace pmm {

    static uint8_t* bitmap; // Bitmap to track allocated/free pages
    static uintptr_t memory_start; // Start of physical memory
    static uintptr_t memory_end;   // End of physical memory
    static size_t total_pages;     // Total number of pages

    // Initialize the physical memory manager with the given memory range
    void init(uintptr_t start, uintptr_t end) {
        // Align start to 4KB boundary
        memory_start = (start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        memory_end = end;
        total_pages = (memory_end - memory_start) / PAGE_SIZE;

        // Allocate bitmap to track page usage
        size_t bitmap_size = (total_pages + 7) / 8; // 1 bit per page
        bitmap = (uint8_t*)memory_start; // Use the beginning of memory for bitmap
        
        // Mark all pages as free initially
        for (size_t i = 0; i < bitmap_size; i++) {
            bitmap[i] = 0;
        }

        // Mark the pages used by the bitmap itself as allocated
        size_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
        for (size_t i = 0; i < bitmap_pages; i++) {
            size_t byte_index = i / 8;
            size_t bit_index = i % 8;
            bitmap[byte_index] |= (1 << bit_index);
        }
    }

    void* alloc_page() {
        for (size_t i = 0; i < total_pages; i++) {
            size_t byte_index = i / 8;
            size_t bit_index = i % 8;

            if ((bitmap[byte_index] & (1 << bit_index)) == 0) { // Page is free
                bitmap[byte_index] |= (1 << bit_index); // Mark page as allocated
                return (void*)(memory_start + i * PAGE_SIZE);
            }
        }
        return nullptr; // No free pages available
    }

    void free_page(void* page) {
        uintptr_t addr = (uintptr_t)page;
        if (addr < memory_start || addr >= memory_end) {
            return; // Invalid page address
        }

        size_t page_index = (addr - memory_start) / PAGE_SIZE;
        size_t byte_index = page_index / 8;
        size_t bit_index = page_index % 8;

        bitmap[byte_index] &= ~(1 << bit_index); // Mark page as free
    }
}