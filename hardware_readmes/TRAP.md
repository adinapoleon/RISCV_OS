# RISC-V Machine-Mode Trap Hardware Specification

## Overview

In RISC-V, a **trap** is the hardware mechanism that transfers control from normal execution to a designated handler. Traps cover two distinct categories:

- **Exceptions** — synchronous events caused by the currently executing instruction (e.g., illegal instruction, page fault, environment call).
- **Interrupts** — asynchronous events from external sources (e.g., timer expiry, UART data ready).

This document covers the **Machine Mode (M-mode)** trap system, which is the highest privilege level and the mode your kernel boots into on QEMU `virt`.

---

## CSR (Control and Status Register) Map

Traps are managed entirely through a set of dedicated **CSRs** — special registers distinct from the 32 general-purpose integer registers. They are accessed with dedicated instructions (`csrr`, `csrw`, `csrrs`, `csrrc`).

| CSR Name  | Address | R/W | Description                                      |
|-----------|---------|-----|--------------------------------------------------|
| `mstatus` | `0x300` | R/W | Global machine status (interrupt enable, privilege) |
| `mie`     | `0x304` | R/W | Machine Interrupt Enable — which interrupts are enabled |
| `mtvec`   | `0x305` | R/W | Machine Trap Vector — base address of trap handler |
| `mscratch`| `0x340` | R/W | Scratch register for trap handler use            |
| `mepc`    | `0x341` | R/W | Machine Exception Program Counter — faulting `pc` |
| `mcause`  | `0x342` | R/W | Cause of the trap (interrupt vs. exception + code) |
| `mtval`   | `0x343` | R/W | Machine Trap Value — additional fault info (bad address, etc.) |
| `mip`     | `0x344` | R   | Machine Interrupt Pending — which interrupts are pending |

> **Analogy to UART:** Just as the UART has `LSR` to read status and `THR` to write data, traps have `mcause` to read what happened and `mtvec` to point at where to go. The CSRs are the "registers" of the trap hardware block.

---

## `mtvec` — Machine Trap Vector Base Address Register

This is the register you **must** set before any trap can fire safely. It holds the address of your handler.

### Bit Layout (RV32)

```
 31                               2   1  0
┌───────────────────────────────────┬──┬──┐
│              BASE                 │  │Mo│
│         (handler address)         │0 │de│
└───────────────────────────────────┴──┴──┘
```

| Field  | Bits  | Description                                              |
|--------|-------|----------------------------------------------------------|
| `BASE` | [31:2]| Base address of the trap handler (must be 4-byte aligned)|
| `MODE` | [1:0] | Trap vectoring mode (see table below)                    |

### MODE Field

| Value | Name     | Behavior                                                                 |
|-------|----------|--------------------------------------------------------------------------|
| `00`  | Direct   | All traps jump to `BASE`. Simplest — use this first.                     |
| `01`  | Vectored | Exceptions go to `BASE`; interrupts go to `BASE + 4 * cause`. Efficient for many interrupt sources. |
| `1x`  | Reserved | Do not use.                                                              |

**Example — setting Direct mode:**
```asm
la   t0, trap_entry   # load address of your handler label
csrw mtvec, t0        # BASE = trap_entry, MODE = 00 (la gives 4-byte aligned addr)
```

---

## `mcause` — Machine Cause Register

Written by hardware when a trap fires. Tells you **what** happened.

### Bit Layout (RV32)

```
 31   30                              0
┌──┬────────────────────────────────────┐
│I │         Exception Code             │
└──┴────────────────────────────────────┘
```

| Field            | Bits   | Description                                              |
|------------------|--------|----------------------------------------------------------|
| Interrupt bit (I)| [31]   | `1` = interrupt, `0` = exception                         |
| Exception Code   | [30:0] | Identifies the specific trap cause (see tables below)    |

### Exception Codes (Interrupt bit = 0)

Synchronous — caused by the instruction at `mepc`.

| Code | Name                           | Typical Cause                                          |
|------|--------------------------------|--------------------------------------------------------|
| 0    | Instruction Address Misaligned | Jump to non-4-byte-aligned address (without C ext)     |
| 1    | Instruction Access Fault       | Fetch from invalid/protected memory                    |
| 2    | Illegal Instruction            | Undefined opcode, wrong CSR access, `unimp`            |
| 3    | Breakpoint                     | `ebreak` instruction                                   |
| 4    | Load Address Misaligned        | Unaligned `lw`/`lh` load                               |
| 5    | Load Access Fault              | Load from invalid memory address                       |
| 6    | Store/AMO Address Misaligned   | Unaligned `sw`/`sh` store                              |
| 7    | Store/AMO Access Fault         | Store to invalid/protected memory                      |
| 8    | Environment Call from U-mode   | `ecall` in User mode                                   |
| 9    | Environment Call from S-mode   | `ecall` in Supervisor mode                             |
| 11   | Environment Call from M-mode   | `ecall` in Machine mode                                |

### Interrupt Codes (Interrupt bit = 1)

Asynchronous — `mepc` points to the instruction that *would have* executed next.

| Code | Name                     | Source                                               |
|------|--------------------------|------------------------------------------------------|
| 3    | Machine Software Interrupt | MSIP bit in CLINT `msip` register                  |
| 7    | Machine Timer Interrupt  | `mtime >= mtimecmp` in the CLINT                     |
| 11   | Machine External Interrupt| PLIC signals an external device (UART, virtio, etc.) |

**Reading `mcause` in C:**
```cpp
uint32_t mcause;
asm volatile("csrr %0, mcause" : "=r"(mcause));

bool is_interrupt  = (mcause >> 31) & 1;
uint32_t code      = mcause & 0x7FFFFFFF;
```

---

## `mepc` — Machine Exception Program Counter

Hardware saves the **faulting (or interrupted) PC** here before jumping to `mtvec`.

| Trap Type  | `mepc` points to...                                                  |
|------------|----------------------------------------------------------------------|
| Exception  | The instruction that **caused** the fault                            |
| Interrupt  | The instruction that **would have executed next** (none have faulted)|

To return from a trap, you use `mret`, which restores `pc ← mepc`. If you want to skip the faulting instruction (e.g., after handling an `ecall`), you must manually advance it:

```cpp
// In your C trap handler — advance past a 4-byte instruction
asm volatile(
    "csrr t0, mepc\n"
    "addi t0, t0, 4\n"
    "csrw mepc, t0\n"
);
```

> **Note:** With the Compressed (C) extension, instructions can be 2 bytes. Since your Makefile uses `-march=rv32im` (no C), all instructions are exactly 4 bytes — safe to always add 4.

---

## `mtval` — Machine Trap Value

Provides additional context depending on the exception type. Hardware writes this; you read it.

| Exception Type              | `mtval` contains                                    |
|-----------------------------|-----------------------------------------------------|
| Instruction fault/misalign  | The faulting instruction address                    |
| Load/Store fault/misalign   | The memory address that was accessed                |
| Illegal instruction         | The faulting instruction word (or 0)                |
| Breakpoint (`ebreak`)       | The address of the `ebreak`                         |
| All other exceptions        | `0` (undefined / not applicable)                   |
| Interrupts                  | `0` (always)                                        |

---

## `mstatus` — Machine Status Register

Controls global interrupt state. The two fields you care about now:

```
 31                    13  12 11   8   7   4   3   0
┌─────────────────────────┬────┬─────┬───┬─────┬───┐
│         ...             │ MPP│ ... │MPIE│ ... │MIE│
└─────────────────────────┴────┴─────┴───┴─────┴───┘
```

| Field | Bits  | Description                                                     |
|-------|-------|-----------------------------------------------------------------|
| `MIE` | [3]   | Machine Interrupt Enable. `1` = interrupts can fire globally    |
| `MPIE`| [7]   | Previous MIE — saved by hardware when a trap fires              |
| `MPP` | [12:11]| Previous Privilege level — saved by hardware, restored by `mret`|

**What hardware does automatically when a trap fires:**
1. `MPIE ← MIE` (saves interrupt enable state)
2. `MIE ← 0` (disables interrupts — nested traps won't fire by default)
3. `MPP ← current privilege` (saves M-mode)
4. `pc ← mtvec`

**What `mret` does:**
1. `MIE ← MPIE` (restores interrupt enable)
2. `privilege ← MPP` (restores privilege level)
3. `pc ← mepc` (resumes execution)

---

## `mie` — Machine Interrupt Enable Register

Bit-mask controlling which *specific* interrupt sources are enabled (in addition to `mstatus.MIE`). Both must be set for an interrupt to fire.

```
 31          12  11   8   7   4   3   0
┌─────────────────┬───┬─────┬───┬─────┬───┐
│       0         │MEIE│ ... │MTIE│ ... │MSIE│
└─────────────────┴───┴─────┴───┴─────┴───┘
```

| Bit | Name   | Controls                        |
|-----|--------|---------------------------------|
| 3   | `MSIE` | Machine Software Interrupt      |
| 7   | `MTIE` | Machine Timer Interrupt         |
| 11  | `MEIE` | Machine External Interrupt      |

**Enabling the timer interrupt:**
```asm
li   t0, (1 << 7)   # MTIE bit
csrs mie, t0        # csrs = CSR set bits
```

---

## `mscratch` — Machine Scratch Register

A free-use CSR — hardware never touches it. Conventionally used to save a register during the trap entry sequence, so you have a free register to work with before saving the full context.

For example, a robust kernel stores the address of a pre-allocated **kernel stack** here, so the trap entry can atomically swap `sp` with `mscratch` and land on a known-good stack:

```asm
csrrw sp, mscratch, sp  # swap sp <-> mscratch; now sp = kernel stack
```

You don't need this yet (your current handler reuses the interrupted stack), but it becomes essential once you add user processes.

---

## The Hardware Trap Sequence (Summary)

This is the complete sequence the **hardware performs automatically** every time a trap fires, before your code runs a single instruction:

```
1.  mepc   ← pc          (save return address)
2.  mcause ← cause        (exception/interrupt + code)
3.  mtval  ← fault info   (bad address or instruction word)
4.  mstatus.MPIE ← mstatus.MIE
5.  mstatus.MIE  ← 0      (disable interrupts)
6.  mstatus.MPP  ← current privilege
7.  pc     ← mtvec        (jump to handler — Direct mode)
```

Your handler's job:

```
1.  Save all 31 general-purpose registers  (hardware does NOT do this)
2.  Read mcause, mepc, mtval
3.  Dispatch to the right C handler
4.  Optionally modify mepc (e.g., advance past ecall)
5.  Restore all 31 registers
6.  mret                                   (hardware restores pc, MIE, privilege)
```

---

## CLINT — Core Local Interruptor (Timer & Software Interrupts)

For the QEMU `virt` machine, the timer is provided by the **CLINT**, memory-mapped at `0x02000000`.

| Address      | Size    | Register     | Description                                      |
|--------------|---------|--------------|--------------------------------------------------|
| `0x02000000` | 4 bytes | `msip`       | Machine Software Interrupt Pending (write 1 to trigger) |
| `0x02004000` | 8 bytes | `mtime`      | Current real-time counter (increments at 10 MHz on virt) |
| `0x02004008` | 8 bytes | `mtimecmp`   | Timer fires when `mtime >= mtimecmp`              |

**Setting a timer interrupt 1 second from now:**
```cpp
volatile uint64_t* mtime    = (volatile uint64_t*)0x02004000;
volatile uint64_t* mtimecmp = (volatile uint64_t*)0x02004008;
*mtimecmp = *mtime + 10000000; // 10 MHz clock → 1 second
```

Then enable the timer interrupt in `mie` and `mstatus.MIE`, and your trap handler will fire with `mcause = 0x80000007`.

---

