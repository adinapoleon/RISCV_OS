#include "uart.h"

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

    uart.print_str("\nDone!\n");

    while (1) {} // infinite loop to prevent the kernel from exiting
}