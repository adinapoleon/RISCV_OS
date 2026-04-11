# RISC-V Simple OS

A bare-metal operating system for the RISC-V architecture, targeting the QEMU `virt` machine.

---

## Current Progress: Machine-Mode Trap Handler

We have implemented a fully working M-mode trap handler on top of the existing UART driver. All traps — exceptions and interrupts — are now caught, diagnosed, and reported over UART.

### Key Features of the Trap Handler:

*   **`mtvec` Installation:** On boot, `start.S` loads the address of `trap_entry` into `mtvec` (Machine Trap Vector Register) before calling `kernel_main`, ensuring all traps are caught from the moment the kernel starts.
*   **Full Register Save/Restore:** The assembly entry point `trap_entry` saves all 31 general-purpose registers (x1–x31) onto the stack as a `TrapFrame` before dispatching to C, and restores them on return. The CPU does not do this automatically.
*   **C Dispatcher:** `trap_handler()` in `trap.cpp` receives `mcause`, `mepc`, `mtval`, and a pointer to the `TrapFrame`. It decodes the interrupt bit (bit 31 of `mcause`) to distinguish interrupts from exceptions, then prints diagnostic information over UART.
*   **`mret` Return:** After the C handler returns, the assembly stub restores all registers and executes `mret`, which atomically restores `pc ← mepc`, re-enables interrupts (`MIE ← MPIE`), and restores privilege level from `mstatus`.
*   **Verified with `IllegalInstruction`:** Tested by emitting a `.word 0x00000000` (guaranteed illegal instruction), which produced the correct output: `cause=0x00000002`, confirming the full pipeline works end-to-end.

### How Traps Work on RISC-V (M-mode):

When any trap fires, the hardware automatically:
1. Saves `pc` → `mepc`
2. Writes the cause → `mcause` (bit 31 = interrupt flag, low bits = code)
3. Writes fault info → `mtval` (bad address, faulting instruction, etc.)
4. Disables interrupts (`mstatus.MIE ← 0`)
5. Jumps to `mtvec`

Your handler is then responsible for saving registers, dispatching, and returning via `mret`.

---

## Step 1: Robust UART Driver

The foundation of all kernel output. Moved beyond a raw memory-mapped pointer to a proper object-oriented driver.

### Key Features:

*   **16550A Compliance:** Uses standard register naming (RBR, THR, LSR, etc.) as defined in the 16550A specification.
*   **Busy-Waiting (Polling):** Polls the **Line Status Register (LSR)** bit 5 (**THRE**) to ensure the transmitter is ready before sending each character, preventing character loss.
*   **Object-Oriented Design:** Encapsulated in a `Uart` class with a clean C++ interface:
    *   `print_char(char c)`: Core primitive for hardware interaction.
    *   `print_str(const char* s)`: Null-terminated string output.
    *   `print_int(int32_t n)`: Custom integer-to-ASCII (no `std` available).
    *   `print_hex(uint32_t n)`: Hex output with `0x` prefix, used extensively in the trap handler.
*   **Modern C++ Practices:** Uses `#pragma once` for header guarding and `volatile` pointers for memory-mapped I/O to prevent incorrect compiler optimizations.
*   **Hardware Initialization:** The `Uart` constructor disables interrupts, enables and clears FIFOs, and sets the 8-n-1 data format.

---

## How to Run

Ensure you have the RISC-V GCC toolchain (`riscv64-unknown-elf-g++`) and QEMU (`qemu-system-riscv32`) installed.

```bash
make run
```

---

## Next Steps

- [x] **Step 1: UART Driver** - 16550A-compliant, polled, object-oriented UART driver.
- [x] **Step 2: Trap & Interrupt Handling** - M-mode trap vector, full register save/restore, C dispatcher, verified with illegal instruction exception.
- [ ] **Step 3: Physical Memory Management** - Creating a page allocator for dynamic memory.
- [ ] **Step 4: Supervisor Mode** - Transitioning from Machine Mode to Supervisor Mode via `mret` into S-mode, delegating traps with `medeleg`/`mideleg`.
- [ ] **Step 5: Timer Interrupt** - Wiring up the CLINT (`mtime`/`mtimecmp`) and handling `mcause=0x80000007`.