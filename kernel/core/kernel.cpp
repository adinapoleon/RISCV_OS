#include "drivers/uart.h"
#include "memory/pmm.h"
#include "memory/vmm.h"

namespace {

extern "C" char _ram_end;

#ifndef KERNEL_RUN_TRAP_TESTS
#define KERNEL_RUN_TRAP_TESTS 0
#endif

static void print_pmm_stats(Uart& uart) {
    uart.print_str("PMM total pages: ");
    uart.print_int((int32_t)pmm::total_pages());
    uart.print_str(", used pages: ");
    uart.print_int((int32_t)pmm::used_pages());
    uart.print_str(", free pages: ");
    uart.print_int((int32_t)pmm::free_pages());
    uart.print_str("\n");
}

static bool print_check(Uart& uart, const char* name, bool passed) {
    uart.print_str(passed ? "[OK] " : "[FAIL] ");
    uart.print_str(name);
    uart.print_str("\n");
    return passed;
}

static bool init_memory(Uart& uart) {
    uart.print_str("\n\n--- Testing Physical Memory Manager ---\n");

    pmm::init_after_kernel(reinterpret_cast<uintptr_t>(&_ram_end));
    print_pmm_stats(uart);

    uart.print_str("\n--- Initializing Virtual Memory Scaffold ---\n");
    if (vmm::init()) {
        uart.print_str("Allocated Sv32 root page table at: ");
        uart.print_hex(vmm::root_table_address());
        uart.print_str("\n");
    } else {
        uart.print_str("Failed to allocate Sv32 root page table!\n");
        return false;
    }
    print_pmm_stats(uart);

    return true;
}

static bool run_pmm_smoke_test(Uart& uart) {
    bool passed = true;
    size_t initial_free_pages = pmm::free_pages();

    uart.print_str("\nAllocating single page of memory...\n");
    void* page1 = pmm::alloc_page();
    if (page1) {
        uart.print_str("Allocated page at address: ");
        uart.print_hex((uintptr_t)page1);
        uart.print_str("\n");
    } else {
        uart.print_str("Failed to allocate page!\n");
    }
    print_pmm_stats(uart);
    passed = print_check(uart, "alloc_page returns a page", page1 != nullptr) && passed;
    passed = print_check(uart, "allocated page is 4KB-aligned", ((uintptr_t)page1 % pmm::PAGE_SIZE) == 0) && passed;
    passed = print_check(uart, "alloc_page consumes one free page", pmm::free_pages() == initial_free_pages - 1) && passed;

    uart.print_str("\nFreeing the allocated page...\n");
    bool freed_page1 = pmm::free_page(page1);
    print_pmm_stats(uart);
    passed = print_check(uart, "free_page reports a valid free", freed_page1) && passed;
    passed = print_check(uart, "free_page restores the free count", pmm::free_pages() == initial_free_pages) && passed;
    passed = print_check(uart, "free_page rejects double frees", !pmm::free_page(page1)) && passed;
    passed = print_check(uart, "free_page rejects invalid pages", !pmm::free_page(reinterpret_cast<void*>(0xDEADBEEF))) && passed;

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
    passed = print_check(uart, "allocator reuses the freed page", page2 == page1) && passed;

    uart.print_str("\nFreeing the reused page...\n");
    bool freed_page2 = pmm::free_page(page2);
    print_pmm_stats(uart);
    passed = print_check(uart, "free_page reports a valid reused-page free", freed_page2) && passed;
    passed = print_check(uart, "PMM smoke test leaves no leaked pages", pmm::free_pages() == initial_free_pages) && passed;

    uart.print_str("\nTesting zeroed page allocation...\n");
    void* dirty_page = pmm::alloc_page();
    if (dirty_page) {
        uint32_t* words = static_cast<uint32_t*>(dirty_page);
        words[0] = 0xA5A5A5A5;
        words[(pmm::PAGE_SIZE / sizeof(uint32_t)) - 1] = 0x5A5A5A5A;
        pmm::free_page(dirty_page);
    }

    void* zeroed_page = pmm::alloc_zeroed_page();
    print_pmm_stats(uart);
    passed = print_check(uart, "alloc_zeroed_page returns a page", zeroed_page != nullptr) && passed;
    if (zeroed_page) {
        uint32_t* words = static_cast<uint32_t*>(zeroed_page);
        passed = print_check(uart, "alloc_zeroed_page clears first word", words[0] == 0) && passed;
        passed = print_check(uart, "alloc_zeroed_page clears last word", words[(pmm::PAGE_SIZE / sizeof(uint32_t)) - 1] == 0) && passed;
    }
    passed = print_check(uart, "alloc_zeroed_page consumes one free page", pmm::free_pages() == initial_free_pages - 1) && passed;

    uart.print_str("\nFreeing the zeroed page...\n");
    bool freed_zeroed_page = pmm::free_page(zeroed_page);
    print_pmm_stats(uart);
    passed = print_check(uart, "free_page reports a valid zeroed-page free", freed_zeroed_page) && passed;
    passed = print_check(uart, "zeroed-page test leaves no leaked pages", pmm::free_pages() == initial_free_pages) && passed;

    uart.print_str(passed ? "\nPMM smoke test passed.\n" : "\nPMM smoke test failed.\n");
    return passed;
}

#if KERNEL_RUN_TRAP_TESTS
static void run_trap_smoke_tests(Uart& uart) {
    uart.print_str("\n\n--- Testing illegal instruction exception ---\n");
    asm volatile(".word 0x00000000");

    uart.print_str("\n\n--- Testing load access fault ---\n");
    {
        volatile uint32_t* bad_ptr = (volatile uint32_t*)0xDEADBEEF;
        volatile uint32_t val = *bad_ptr;
        (void)val;
    }

    uart.print_str("\n\n--- Testing store access fault ---\n");
    {
        volatile uint32_t* bad_ptr = (volatile uint32_t*)0xDEADBEEF;
        *bad_ptr = 0x42;
    }

    uart.print_str("\n\n--- Testing breakpoint ---\n");
    asm volatile("ebreak");

    uart.print_str("\n\nTrap smoke tests returned to kernel_main.\n");
}
#endif

}

// to avoid name mangling
extern "C" void kernel_main() {
    Uart uart;

    uart.print_str("\n\n--- RISC-V OS Kernel ---\n");
    uart.print_str("Booting supervisor kernel...\n");

    if (init_memory(uart) && run_pmm_smoke_test(uart)) {
        uart.print_str("\nMemory bring-up complete.\n");
    }

#if KERNEL_RUN_TRAP_TESTS
    run_trap_smoke_tests(uart);
#endif

    uart.print_str("\nKernel idle.\n");

    while (1) {}
}
