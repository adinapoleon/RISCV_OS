#include "kernel/trap.h"
#include "drivers/uart.h"

// for now just print the trap info and advance mepc to avoid infinite traps
static void advance_mepc() {
    asm volatile(
        "csrr t0, mepc\n"
        "addi t0, t0, 4\n"
        "csrw mepc, t0\n"
        :
        :
        : "t0"
    );
}

void handle_interrupt(uint32_t code, uint32_t mepc, uint32_t mtval, TrapFrame* frame) {
    (void)mepc;
    (void)mtval;
    (void)frame;
    Uart uart;

    switch (static_cast<InterruptCode>(code)) {
        case InterruptCode::MachineSoftwareInterrupt:
            uart.print_str("[INTERRUPT] Machine Software Interrupt\n");
            advance_mepc();
            break;

        case InterruptCode::MachineTimerInterrupt:
            uart.print_str("[INTERRUPT] Machine Timer Interrupt\n");
            advance_mepc();
            break;

        case InterruptCode::MachineExternalInterrupt:
            uart.print_str("[INTERRUPT] Machine External Interrupt\n");
            advance_mepc();
            break;

        default:
            uart.print_str("[INTERRUPT] code=");
            uart.print_hex(code);
            uart.print_str(" mepc=");
            uart.print_hex(mepc);
            uart.print_str(" mtval=");
            uart.print_hex(mtval);
            uart.print_str("\n");
            advance_mepc();
            break;
    }
}

void handle_exception(uint32_t code, uint32_t mepc, uint32_t mtval, TrapFrame* frame) {
    (void)mepc;
    (void)mtval;
    (void)frame;
    Uart uart;

    switch (static_cast<ExceptionCode>(code)) {
        case ExceptionCode::IllegalInstruction:
            uart.print_str("[EXCEPTION] Illegal Instruction\n");
            advance_mepc();
            break;

        case ExceptionCode::Breakpoint:
            uart.print_str("[EXCEPTION] Breakpoint\n");
            advance_mepc();
            break;

        case ExceptionCode::LoadAccessFault:
        case ExceptionCode::StoreAMOAccessFault:
            uart.print_str("[EXCEPTION] Load Access Fault\n");
            advance_mepc();
            break;

        case ExceptionCode::EnvironmentCallFromUMode:
        case ExceptionCode::EnvironmentCallFromSMode:
        case ExceptionCode::EnvironmentCallFromMMode:
            uart.print_str("[EXCEPTION] Environment Call\n");
            advance_mepc();
            break;

        case ExceptionCode::InstructionPageFault:
        case ExceptionCode::LoadPageFault:
        case ExceptionCode::StoreAMOPageFault:
            uart.print_str("[EXCEPTION] Page Fault\n");
            advance_mepc();
            break;

        default:
            uart.print_str("[EXCEPTION] code=");
            uart.print_hex(code);
            uart.print_str(" mepc=");
            uart.print_hex(mepc);
            uart.print_str(" mtval=");
            uart.print_hex(mtval);
            uart.print_str("\n");
            advance_mepc();
            break;
    }
}

extern "C" void trap_handler(uint32_t mcause, uint32_t mepc, uint32_t mtval, TrapFrame* frame) {
    bool is_interrupt = (mcause >> 31) & 1;
    uint32_t code = mcause & 0x7FFFFFFF;

    if (is_interrupt) {
        handle_interrupt(code, mepc, mtval, frame);
    } else {
        handle_exception(code, mepc, mtval, frame);
    }


}
