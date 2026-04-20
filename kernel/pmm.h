#pragma once
#include <stdint.h>

// Physical Memory Manager (PMM) for RISCV OS
namespace pmm {
    const uint32_t PAGE_SIZE = 4096; // 4KB pages, RISCV standatd page size

    void init(uintptr_t start, size_t end); // Initialize the physical memory manager

    void* alloc_page(); // Allocate a single page of physical memory

    void free_page(void* page); 
    // Free a previously allocated page of physical memory
}