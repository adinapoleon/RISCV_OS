#pragma once

#include <stdint.h>

struct TrapFrame;

namespace syscall {
    constexpr uint32_t SYS_putchar = 1;
    constexpr uint32_t SYS_exit = 2;

    bool exit_requested();
    uint32_t exit_code();
    void handle(TrapFrame* frame);
}
