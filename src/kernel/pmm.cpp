#include "kernel/pmm.h"
#include "kernel/mem.h"
#include <stddef.h>

// Simple bitmap-based physical memory manager implementation
namespace pmm {

    static uint8_t* bitmap; // Bitmap to track allocated/free pages
    static uintptr_t memory_start; // Start of physical memory
    static uintptr_t memory_end;   // End of physical memory
    static size_t total_page_count; // Total number of pages

    static bool is_page_allocated(size_t page_index) {
        size_t byte_index = page_index / 8;
        size_t bit_index = page_index % 8;
        return (bitmap[byte_index] & (1 << bit_index)) != 0;
    }

    static void set_page_allocated(size_t page_index, bool allocated) {
        size_t byte_index = page_index / 8;
        size_t bit_index = page_index % 8;

        if (allocated) {
            bitmap[byte_index] |= (1 << bit_index);
        } else {
            bitmap[byte_index] &= ~(1 << bit_index);
        }
    }

    // Initialize the physical memory manager with the given memory range
    void init(uintptr_t start, uintptr_t end) {
        // Align start to 4KB boundary
        memory_start = (start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        memory_end = end & ~(PAGE_SIZE - 1);
        if (memory_start >= memory_end) {
            bitmap = nullptr;
            total_page_count = 0;
            return;
        }

        total_page_count = (memory_end - memory_start) / PAGE_SIZE;

        // Allocate bitmap to track page usage
        size_t bitmap_size = (total_page_count + 7) / 8; // 1 bit per page
        bitmap = (uint8_t*)memory_start; // Use the beginning of memory for bitmap
        
        // Mark all pages as free initially
        for (size_t i = 0; i < bitmap_size; i++) {
            bitmap[i] = 0;
        }

        // Mark the pages used by the bitmap itself as allocated
        size_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
        for (size_t i = 0; i < bitmap_pages; i++) {
            set_page_allocated(i, true);
        }
    }

    void* alloc_page() {
        for (size_t i = 0; i < total_page_count; i++) {
            if (!is_page_allocated(i)) {
                set_page_allocated(i, true);
                return (void*)(memory_start + i * PAGE_SIZE);
            }
        }
        return nullptr; // No free pages available
    }

    void* alloc_zeroed_page() {
        void* page = alloc_page();
        if (page != nullptr) {
            mem::zero(page, PAGE_SIZE);
        }

        return page;
    }

    void free_page(void* page) {
        uintptr_t addr = (uintptr_t)page;
        if (addr < memory_start || addr >= memory_end || (addr % PAGE_SIZE) != 0) {
            return; // Invalid page address
        }

        size_t page_index = (addr - memory_start) / PAGE_SIZE;
        if (!is_page_allocated(page_index)) {
            return; // Double free
        }

        set_page_allocated(page_index, false);
    }

    size_t total_pages() {
        return total_page_count;
    }

    size_t free_pages() {
        size_t count = 0;
        for (size_t i = 0; i < total_page_count; i++) {
            if (!is_page_allocated(i)) {
                count++;
            }
        }

        return count;
    }

    size_t used_pages() {
        return total_page_count - free_pages();
    }
}
