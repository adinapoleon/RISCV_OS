#include "trap.h"
#include "uart.h"

extern "C" void trap_handler(uint32_t mcause, uint32_t mepc, uint32_t mtval, TrapFrame* frame) {
    
    (void)frame;
    Uart uart;

    bool is_interrupt = (mcause >> 31) & 1;
    uint32_t code = mcause & 0x7FFFFFFF;

    if (is_interrupt) {
        uart.print_str("[INTERRUPT] cause=");
        uart.print_hex(code);
        uart.print_str("\n");
    } else {
        uart.print_str("[EXCEPTION] cause=");
        uart.print_hex(code);
        uart.print_str(" mepc=");
        uart.print_hex(mepc);
        uart.print_str(" mtval=");
        uart.print_hex(mtval);
        uart.print_str("\n");

        // advace past faulty instruction to prevent infinite loop of exceptions
        asm volatile(
            "csrr t0, mepc\n" // read mepc into t0
            "addi t0, t0, 4\n" // advance to next instruction 
            "csrw mepc, t0\n"  // write back to mepc
        );

        return;
    }
}