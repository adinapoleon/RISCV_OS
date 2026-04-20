# RISC-V Simple OS

A bare-metal operating system for the RISC-V architecture, targeting the QEMU `virt` machine.

---

## Current Progress: Physical Memory Management

We have evolved the kernel from basic hardware interaction to a system capable of managing its own resources. The kernel now includes a robust Physical Memory Manager (PMM) and a clear roadmap for future development.

### Key Features of the Physical Memory Manager (PMM):

*   **Bitmap-Based Allocation:** Uses a bitmap to track the status of every 4KB page in the 128MB RAM provided by QEMU.
*   **Linker Integration:** Automatically detects the start of available memory using the `_end` symbol from the linker script, ensuring the kernel never overwrites its own code or static data.
*   **Page Alignment:** Enforces strict 4KB alignment for all allocated pages, a prerequisite for RISC-V hardware paging (Sv32).
*   **Self-Protection:** The allocator is "aware" of the memory it uses for its own bitmap and marks those pages as reserved to prevent self-corruption.
*   **Standard Interface:** Provides a clean `alloc_page()` and `free_page()` API, abstracting the complexity of bit-manipulation from the rest of the kernel.

---

## Step 2: Machine-Mode Trap Handler

A fully working M-mode trap handler that catches, diagnoses, and reports all hardware exceptions and interrupts.

*   **`mtvec` Installation:** Assembly entry point `trap_entry` is installed on boot.
*   **Full Context Save:** Saves all 31 general-purpose registers (x1–x31) as a `TrapFrame`.
*   **C Dispatcher:** Decodes `mcause` to distinguish interrupts from exceptions and prints diagnostics.
*   **Verified Exceptions:** Tested with Illegal Instruction, Load/Store faults, and Breakpoints.

---

## Step 1: Robust UART Driver

The foundation of all kernel output, providing 16550A-compliant serial communication.

*   **Polled I/O:** Uses the Line Status Register (LSR) to ensure hardware readiness.
*   **Object-Oriented:** Encapsulated in a `Uart` class with support for strings, integers, and hexadecimal output.

---

## Project Roadmap

1.  **Phase 1: Stabilization & S-Mode** (In Progress)
    - [x] Physical Memory Manager
    - [ ] Transition from Machine Mode (M) to Supervisor Mode (S)
    - [ ] Interrupt-driven UART (moving away from polling)
2.  **Phase 2: Virtual Memory**
    - [ ] Sv32 Page Table implementation
    - [ ] Identity mapping for Kernel
    - [ ] Kernel Heap (kmalloc)
3.  **Phase 3: Processes & Scheduling**
    - [ ] Context switching assembly logic
    - [ ] Timer-based preemption (CLINT)
    - [ ] Round-robin scheduler
4.  **Phase 4: User Space**
    - [ ] `ecall` based System Call interface
    - [ ] Privilege separation (U-mode)
    - [ ] Basic `write` and `exit` syscalls

---

## Documentation

Detailed specifications for the hardware components and software abstractions can be found in the `notes/` directory:
*   `notes/TRAP.md`: Machine-mode trap handling and CSRs.
*   `notes/UART.md`: 16550A UART register map and initialization.
*   `notes/MEMORY.md`: Physical and Virtual memory management (Sv32).

---

## How to Run

Ensure you have the RISC-V GCC toolchain (`riscv64-unknown-elf-g++`) and QEMU (`qemu-system-riscv32`) installed.

```bash
make run
```
---
