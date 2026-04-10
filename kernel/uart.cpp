#include "uart.h"

Uart::Uart() {
    // Basic initialization for 16550A UART
    
    // 1. Disable all interrupts
    *reg(IER) = 0x00;
    // 2. Enable FIFOs, clear them
    *reg(FCR) = 0x07;
    // 3. Set word length to 8-bits, no parity, 1 stop bit
    *reg(LCR) = 0x03;
}

void Uart::print_char(char c) {
    // Loop until the Transmitter Holding Register is empty (LSR bit 5)
    while ((*reg(LSR) & LSR_TX_IDLE) == 0) {
        // busy wait
    }
    // Write character to the THR
    *reg(THR) = static_cast<uint8_t>(c);
}

void Uart::print_str(const char* s) {
    for (int i = 0; s[i] != '\0'; i++) {
        print_char(s[i]);
    }
}

void Uart::print_int(int32_t n) {
    if (n == 0) {
        print_char('0');
        return;
    }

    if (n < 0) {
        print_char('-');
        n = -n;
    }

    // Temporary buffer to hold digits (max 10 for int32_t)
    char buffer[11];
    int i = 0;

    while (n > 0) {
        buffer[i++] = (n % 10) + '0';
        n /= 10;
    }

    // Print digits in reverse order
    while (i > 0) {
        print_char(buffer[--i]);
    }
}

void Uart::print_hex(uint32_t n) {
    // Print 0x prefix
    print_char('0');
    print_char('x');
    
    // Print 8 hex digits with leading zeros
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t digit = (n >> i) & 0xF;
        if (digit < 10) {
            print_char(digit + '0');
        } else {
            print_char((digit - 10) + 'A');
        }
    }
}