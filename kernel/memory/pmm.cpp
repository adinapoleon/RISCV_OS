#include "memory/pmm.h"
#include "memory/mem.h"
#include <stddef.h>

extern "C" char _end;

// Simple bitmap-based physical memory manager implementation
namespace pmm {

    static uint8_t* bitmap; // Bitmap to track allocated/free pages
    static uintptr_t memory_start; // Start of physical memory
    static uintptr_t memory_end;   // End of physical memory
    static size_t total_page_count; // Total number of pages

    static uintptr_t align_up(uintptr_t value, uintptr_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    static uintptr_t align_down(uintptr_t value, uintptr_t alignment) {
        return value & ~(alignment - 1);
    }

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

    static void reserve_pages(size_t first_page, size_t page_count) {
        for (size_t i = 0; i < page_count && first_page + i < total_page_count; i++) {
            set_page_allocated(first_page + i, true);
        }
    }

    // Initialize the physical memory manager with the given memory range
    void init(uintptr_t start, uintptr_t end) {
        // Align start to 4KB boundary
        memory_start = align_up(start, PAGE_SIZE);
        memory_end = align_down(end, PAGE_SIZE);
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
        reserve_pages(0, bitmap_pages);
    }

    void init_after_kernel(uintptr_t ram_end) {
        init((uintptr_t)&_end, ram_end);
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
