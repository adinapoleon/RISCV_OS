# RISC-V Simple OS

A bare-metal operating system for the RISC-V architecture, targeting the QEMU `virt` machine.

## Current Progress: Robust UART Driver

We have moved beyond a simple memory-mapped pointer to a robust, object-oriented UART driver.

### Key Features of the Driver:
*   **16550A Compliance:** Uses standard register naming (RBR, THR, LSR, etc.) as defined in the 16550A specification.
*   **Busy-Waiting (Polling):** Instead of "firing and forgetting," the driver polls the **Line Status Register (LSR)** bit 5 (**THRE**) to ensure the transmitter is ready before sending a character. This prevents character loss during high-speed printing.
*   **Object-Oriented Design:** Encapsulated in a `Uart` class with a clean C++ interface:
    *   `print_char(char c)`: The core primitive for hardware interaction.
    *   `print_str(const char* s)`: For string output.
    *   `print_int(int32_t n)`: Custom implementation of integer-to-ASCII conversion (since `std` is not available).
*   **Modern C++ Practices:** Uses `#pragma once` for header guarding and `volatile` pointers for memory-mapped I/O to prevent incorrect compiler optimizations.
*   **Hardware Initialization:** The `Uart` constructor handles the initial configuration of the hardware (disabling interrupts, enabling FIFOs, and setting the 8-n-1 data format).

## How to Run
Ensure you have the RISC-V GCC toolchain and QEMU installed.

```bash
make run
```

## Next Steps
- [ ] **Step 2: Trap & Interrupt Handling** - Implementing the exception/interrupt vector table.
- [ ] **Step 3: Physical Memory Management** - Creating a page allocator for dynamic memory.
- [ ] **Step 4: Supervisor Mode** - Transitioning from Machine Mode to Supervisor Mode.
