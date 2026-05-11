#pragma once
#include <stdint.h>
#include <stddef.h>

// Physical Memory Manager (PMM) for RISCV OS
namespace pmm {
    const uint32_t PAGE_SIZE = 4096; // 4KB pages, RISC-V standard page size

    void init(uintptr_t start, uintptr_t end); // Initialize the physical memory manager

    void init_after_kernel(uintptr_t ram_end);

    void* alloc_page(); // Allocate a single page of physical memory

    void* alloc_zeroed_page(); // Allocate a page and clear it to zero

    bool free_page(void* page);
    // Free a previously allocated page of physical memory. Returns false for invalid or double frees.

    size_t total_pages(); // Total pages managed by the PMM

    size_t free_pages(); // Pages currently available for allocation

    size_t used_pages(); // Pages currently reserved or allocated
}
