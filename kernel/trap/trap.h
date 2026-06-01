#pragma once
#include <stdint.h>

struct TrapFrame {
    uint32_t regs[31]; // x1 -x31 (x0 is hardwired to zero)
};

// Trap cause codes for exceptions and interrupts.
enum class ExceptionCode : uint32_t {
    InstructionAddressMisaligned = 0,
    InstructionAccessFault = 1,
    IllegalInstruction = 2,
    Breakpoint = 3,
    LoadAddressMisaligned = 4,
    LoadAccessFault = 5,
    StoreAMOAddressMisaligned = 6,
    StoreAMOAccessFault = 7,
    EnvironmentCallFromUMode = 8,
    EnvironmentCallFromSMode = 9,
    EnvironmentCallFromMMode = 11,
    InstructionPageFault = 12,
    LoadPageFault = 13,
    StoreAMOPageFault = 15
};

// Interrupt cause codes. Machine and supervisor causes use different numbers.
enum class InterruptCode : uint32_t {
    SupervisorSoftwareInterrupt = 1,
    MachineSoftwareInterrupt = 3,
    SupervisorTimerInterrupt = 5,
    MachineTimerInterrupt = 7,
    SupervisorExternalInterrupt = 9,
    MachineExternalInterrupt = 11
};

void handle_interrupt(uint32_t code, uint32_t epc, uint32_t tval, TrapFrame* frame);
void handle_exception(uint32_t code, uint32_t epc, uint32_t tval, TrapFrame* frame);

namespace trap {
    void begin_privilege_probe();
    bool privilege_probe_trapped();
}

// Trap dispatchers.
extern "C" void supervisor_trap_handler(uint32_t scause, uint32_t sepc, uint32_t stval, TrapFrame* frame);
extern "C" void machine_trap_handler(uint32_t mcause, uint32_t mepc, uint32_t mtval, TrapFrame* frame);
