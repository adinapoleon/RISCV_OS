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
    uart.print_str("\nDone!\n");

    while (1) {} // infinite loop to prevent the kernel from exiting
}