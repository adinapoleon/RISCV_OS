#include "trap/trap.h"
#include "drivers/uart.h"

static void advance_sepc() {
    asm volatile(
        "csrr t0, sepc\n"
        "addi t0, t0, 4\n"
        "csrw sepc, t0\n"
        :
        :
        : "t0"
    );
}

static void print_trap_values(const char* epc_name, uint32_t epc, const char* tval_name, uint32_t tval) {
    Uart uart;
    uart.print_str(" ");
    uart.print_str(epc_name);
    uart.print_str("=");
    uart.print_hex(epc);
    uart.print_str(" ");
    uart.print_str(tval_name);
    uart.print_str("=");
    uart.print_hex(tval);
    uart.print_str("\n");
}

void handle_interrupt(uint32_t code, uint32_t epc, uint32_t tval, TrapFrame* frame) {
    (void)epc;
    (void)tval;
    (void)frame;
    Uart uart;

    switch (static_cast<InterruptCode>(code)) {
        case InterruptCode::SupervisorSoftwareInterrupt:
            uart.print_str("[INTERRUPT] Supervisor Software Interrupt\n");
            break;

        case InterruptCode::MachineSoftwareInterrupt:
            uart.print_str("[INTERRUPT] Machine Software Interrupt\n");
            break;

        case InterruptCode::SupervisorTimerInterrupt:
            uart.print_str("[INTERRUPT] Supervisor Timer Interrupt\n");
            break;

        case InterruptCode::MachineTimerInterrupt:
            uart.print_str("[INTERRUPT] Machine Timer Interrupt\n");
            break;

        case InterruptCode::SupervisorExternalInterrupt:
            uart.print_str("[INTERRUPT] Supervisor External Interrupt\n");
            break;

        case InterruptCode::MachineExternalInterrupt:
            uart.print_str("[INTERRUPT] Machine External Interrupt\n");
            break;

        default:
            uart.print_str("[INTERRUPT] code=");
            uart.print_hex(code);
            print_trap_values("epc", epc, "tval", tval);
            break;
    }
}

void handle_exception(uint32_t code, uint32_t epc, uint32_t tval, TrapFrame* frame) {
    (void)epc;
    (void)tval;
    (void)frame;
    Uart uart;

    switch (static_cast<ExceptionCode>(code)) {
        case ExceptionCode::IllegalInstruction:
            uart.print_str("[EXCEPTION] Illegal Instruction\n");
            advance_sepc();
            break;

        case ExceptionCode::Breakpoint:
            uart.print_str("[EXCEPTION] Breakpoint\n");
            advance_sepc();
            break;

        case ExceptionCode::LoadAccessFault:
            uart.print_str("[EXCEPTION] Load Access Fault\n");
            advance_sepc();
            break;

        case ExceptionCode::StoreAMOAccessFault:
            uart.print_str("[EXCEPTION] Store/AMO Access Fault\n");
            advance_sepc();
            break;

        case ExceptionCode::EnvironmentCallFromUMode:
        case ExceptionCode::EnvironmentCallFromSMode:
        case ExceptionCode::EnvironmentCallFromMMode:
            uart.print_str("[EXCEPTION] Environment Call\n");
            advance_sepc();
            break;

        case ExceptionCode::InstructionPageFault:
        case ExceptionCode::LoadPageFault:
        case ExceptionCode::StoreAMOPageFault:
            uart.print_str("[EXCEPTION] Page Fault\n");
            advance_sepc();
            break;

        default:
            uart.print_str("[EXCEPTION] code=");
            uart.print_hex(code);
            print_trap_values("epc", epc, "tval", tval);
            advance_sepc();
            break;
    }
}

extern "C" void supervisor_trap_handler(uint32_t scause, uint32_t sepc, uint32_t stval, TrapFrame* frame) {
    bool is_interrupt = (scause >> 31) & 1;
    uint32_t code = scause & 0x7FFFFFFF;

    if (is_interrupt) {
        handle_interrupt(code, sepc, stval, frame);
    } else {
        handle_exception(code, sepc, stval, frame);
    }
}

extern "C" void machine_trap_handler(uint32_t mcause, uint32_t mepc, uint32_t mtval, TrapFrame* frame) {
    (void)frame;
    Uart uart;
    uart.print_str("[M-MODE TRAP] cause=");
    uart.print_hex(mcause);
    print_trap_values("mepc", mepc, "mtval", mtval);
    while (1) {}
}
