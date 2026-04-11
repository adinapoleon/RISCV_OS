#include "uart.h"

// to avoid name mangling
extern "C" void kernel_main() {

    // Initialize the UART
    Uart uart;

    // string that will be printed to the UART
    uart.print_str("Hello world from RISC-V OS with a robust UART driver!\n");

    // demonstrating integer printing
    uart.print_str("Let's count: ");
    for (int i = 1; i <= 5; i++) {
        uart.print_int(i);
        if (i < 5) {
            uart.print_str(", ");
        }
    }

    uart.print_str("\nAll hex numbers from 0 to F: ");
    for (uint32_t n = 0x0; n <= 0xF; n++) {
        uart.print_hex(n);
        if (n < 0xF) {
            uart.print_str(", ");
        }
    }

    // Test 1: exception — illegal instruction
    uart.print_str("\n--- Testing illegal instruction exception ---\n");
    asm volatile(".word 0x00000000");

    //add more prints here
    
    uart.print_str("\nDone!\n");

    while (1) {} // infinite loop to prevent the kernel from exiting
}