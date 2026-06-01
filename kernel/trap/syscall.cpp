#include "trap/syscall.h"
#include "trap/trap.h"
#include "drivers/uart.h"

namespace {
constexpr uint32_t REG_A0 = 9;
constexpr uint32_t REG_A7 = 16;

volatile bool recorded_exit = false;
volatile uint32_t recorded_exit_code = 0;
}

namespace syscall {
    bool exit_requested() {
        return recorded_exit;
    }

    uint32_t exit_code() {
        return recorded_exit_code;
    }

    void handle(TrapFrame* frame) {
        if (frame == nullptr) {
            return;
        }

        uint32_t number = frame->regs[REG_A7];
        uint32_t arg0 = frame->regs[REG_A0];

        switch (number) {
            case SYS_putchar: {
                Uart uart;
                uart.print_char(static_cast<char>(arg0));
                frame->regs[REG_A0] = 0;
                break;
            }

            case SYS_exit:
                recorded_exit = true;
                recorded_exit_code = arg0;
                frame->regs[REG_A0] = 0;
                break;

            default:
                frame->regs[REG_A0] = static_cast<uint32_t>(-1);
                break;
        }
    }
}
