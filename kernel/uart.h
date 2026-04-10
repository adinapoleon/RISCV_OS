#pragma once

#include <stdint.h>

class Uart {
public:
    // QEMU 'virt' machine UART base address
    static const uintptr_t BASE = 0x10000000;

    // Register offsets
    enum Register {
        RBR = 0, // Receiver Buffer Register (Read Only)
        THR = 0, // Transmitter Holding Register (Write Only)
        IER = 1, // Interrupt Enable Register
        FCR = 2, // FIFO Control Register
        IIR = 2, // Interrupt Identification Register (Read Only)
        LCR = 3, // Line Control Register
        MCR = 4, // Modem Control Register
        LSR = 5, // Line Status Register
        MSR = 6, // Modem Status Register
        SCR = 7, // Scratch Register
    };

    // LSR Bits
    static const uint8_t LSR_TX_IDLE = 1 << 5;

    Uart();

    // Send a single character, waiting if busy
    void print_char(char c);

    // Print a null-terminated string
    void print_str(const char* s);

    // Print a 32-bit signed integer
    void print_int(int32_t n);

    // Print a hexadecimal number
    void print_hex(uint32_t n);

private:
    // Helper to get a pointer to a specific register
    volatile uint8_t* reg(Register r) {
        return reinterpret_cast<volatile uint8_t*>(BASE + r);
    }
};
