#include "arch/riscv/timer.h"
#include "drivers/uart.h"

namespace {
constexpr uintptr_t CLINT_BASE = 0x02000000;
constexpr uintptr_t CLINT_MTIMECMP = CLINT_BASE + 0x4000;
constexpr uintptr_t CLINT_MTIME = CLINT_BASE + 0xBFF8;
constexpr uint64_t DISABLED_INTERVAL = 0xFFFFFFFFFFFFFFFFull;

volatile uint32_t tick_count = 0;
uint32_t tick_interval = 0;

volatile uint32_t* reg32(uintptr_t address) {
    return reinterpret_cast<volatile uint32_t*>(address);
}

uint64_t read_mtime() {
    volatile uint32_t* mtime_low = reg32(CLINT_MTIME);
    volatile uint32_t* mtime_high = reg32(CLINT_MTIME + 4);

    uint32_t high_before = 0;
    uint32_t low = 0;
    uint32_t high_after = 0;
    do {
        high_before = *mtime_high;
        low = *mtime_low;
        high_after = *mtime_high;
    } while (high_before != high_after);

    return (static_cast<uint64_t>(high_after) << 32) | low;
}

void write_mtimecmp(uint64_t value) {
    volatile uint32_t* mtimecmp_low = reg32(CLINT_MTIMECMP);
    volatile uint32_t* mtimecmp_high = reg32(CLINT_MTIMECMP + 4);

    *mtimecmp_high = 0xFFFFFFFFu;
    *mtimecmp_low = static_cast<uint32_t>(value);
    *mtimecmp_high = static_cast<uint32_t>(value >> 32);
}

void schedule_next_tick() {
    if (tick_interval == 0) {
        write_mtimecmp(DISABLED_INTERVAL);
        return;
    }

    write_mtimecmp(read_mtime() + tick_interval);
}
}

namespace timer {
    void start(uint32_t interval) {
        tick_count = 0;
        tick_interval = interval;
        schedule_next_tick();
    }

    uint32_t ticks() {
        return tick_count;
    }
}

extern "C" void machine_timer_init() {
    write_mtimecmp(DISABLED_INTERVAL);

    constexpr uint32_t MIE_MTIE = 1u << 7;
    asm volatile("csrs mie, %0" : : "r"(MIE_MTIE) : "memory");
}

extern "C" void machine_timer_tick() {
    tick_count++;

    if (tick_count <= 3) {
        Uart uart;
        uart.print_str("Timer tick ");
        uart.print_int(static_cast<int32_t>(tick_count));
        uart.print_str("\n");
    }

    schedule_next_tick();
}
