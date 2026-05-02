#pragma once
#include <stdint.h>

struct TrapFrame {
    uint32_t regs[31]; // x1 -x31 (x0 is hardwired to zero)
};

//mcause codes for exceptions and interrupts
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
    EnvironmentCallFromMMode = 11
};

//mcause codes for interrupts
enum class InterruptCode : uint32_t {
    MachineSoftwareInterrupt = 3,
    MachineTimerInterrupt = 7,
    MachineExternalInterrupt = 11
};

static void handle_interrupt(uint32_t code, uint32_t mepc, uint32_t mtval, TrapFrame* frame);
static void handle_exception(uint32_t code, uint32_t mepc, uint32_t mtval, TrapFrame* frame);

//trap dispatcher
extern "C" void trap_handler(uint32_t mcause, uint32_t mepc, uint32_t mtval, TrapFrame* frame);