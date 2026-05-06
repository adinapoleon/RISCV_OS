#include "drivers/uart.h"
#include "kernel/pmm.h"
#include "kernel/vmm.h"

extern "C" char _end; // Symbol defined by the linker, marks the end of the kernel code and data 

static void print_pmm_stats(Uart& uart) {
    uart.print_str("PMM total pages: ");
    uart.print_int((int32_t)pmm::total_pages());
    uart.print_str(", used pages: ");
    uart.print_int((int32_t)pmm::used_pages());
    uart.print_str(", free pages: ");
    uart.print_int((int32_t)pmm::free_pages());
    uart.print_str("\n");
}

// to avoid name mangling
extern "C" void kernel_main() {

    // Initialize the UART
    Uart uart;

    //add some buffer to differentiate the output
    uart.print_str("\n\n--- Kernel Initialized ---\n");

    // string that will be printed to the UART
    uart.print_str("\nHello world from RISC-V OS with a robust UART driver!\n");

    // demonstrating integer printing
    uart.print_str("\nLet's count: ");
    for (int i = 1; i <= 5; i++) {
        uart.print_int(i);
        if (i < 5) {
            uart.print_str(", ");
        }
    }

    uart.print_str("\n\nAll hex numbers from 0 to F: ");
    for (uint32_t n = 0x0; n <= 0xF; n++) {
        uart.print_hex(n);
        if (n < 0xF) {
            uart.print_str(", ");
        }
    }

    // Test 1: exception — illegal instruction
    uart.print_str("\n\n--- Testing illegal instruction exception ---\n");
    asm volatile(".word 0x00000000");
    
    // Load access fault (cause=0x5)
    uart.print_str("\n\n--- Testing load access fault ---\n");
    {
        volatile uint32_t* bad_ptr = (volatile uint32_t*)0xDEADBEEF;
        volatile uint32_t val = *bad_ptr;
        (void)val;
    }

    // Store access fault (cause=0x7)
    uart.print_str("\n\n--- Testing store access fault ---\n");
    {
        volatile uint32_t* bad_ptr = (volatile uint32_t*)0xDEADBEEF;
        *bad_ptr = 0x42;
    }

    // Breakpoint (cause=0x3) — ebreak instruction
    uart.print_str("\n\n--- Testing breakpoint ---\n");
    asm volatile("ebreak");

    //test if it returns to the main function after handling the interrupt
    uart.print_str("\n\nIf you see this message after the interrupt, it means the kernel successfully handled the interrupt and returned to main.\n");

    // testing memory management by allocating and freeing pages
    uart.print_str("\n\n--- Testing Physical Memory Manager ---\n");

    // 128 MB is default for QEMU virt
    uintptr_t ram_end = 0x80000000 + (128 * 1024 * 1024); // 128 MB
    pmm::init((uintptr_t)&_end, ram_end); // Initialize PMM with memory range after the kernel
    print_pmm_stats(uart);

    uart.print_str("\n--- Initializing Virtual Memory Scaffold ---\n");
    if (vmm::init()) {
        uart.print_str("Allocated Sv32 root page table at: ");
        uart.print_hex(vmm::root_table_address());
        uart.print_str("\n");
    } else {
        uart.print_str("Failed to allocate Sv32 root page table!\n");
    }
    print_pmm_stats(uart);

    uart.print_str("\nAllocating singe page of memory...\n");
    void* page1 = pmm::alloc_page();
    if (page1) {
        uart.print_str("Allocated page at address: ");
        uart.print_hex((uintptr_t)page1);
        uart.print_str("\n");
    } else {
        uart.print_str("Failed to allocate page!\n");
    }
    print_pmm_stats(uart);

    uart.print_str("\nFreeing the allocated page...\n");
    pmm::free_page(page1);
    print_pmm_stats(uart);

    uart.print_str("\nTrying to allocate another page after freeing...\n");
    void* page2 = pmm::alloc_page();
    if (page2) {
        uart.print_str("Allocated page at address: ");
        uart.print_hex((uintptr_t)page2);
        uart.print_str("\n");
    } else {
        uart.print_str("Failed to allocate page!\n");
    }
    print_pmm_stats(uart);

    uart.print_str("\nDone!\n");

    while (1) {} // infinite loop to prevent the kernel from exiting
}
