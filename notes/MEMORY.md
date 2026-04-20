# RISC-V Memory Management Specification (RV32)

## Overview

Memory management in a RISC-V OS is divided into two distinct layers:
1.  **Physical Memory Management (PMM):** Managing raw RAM in 4KB chunks (pages).
2.  **Virtual Memory Management (VMM):** Using the Memory Management Unit (MMU) to map virtual addresses to physical ones via page tables.

This document focuses on the **Sv32** paging scheme used by 32-bit RISC-V systems.

---

## The Physical Memory Map (QEMU `virt`)

On the QEMU `virt` machine, the memory layout is fixed:

| Range                  | Description                                      |
|------------------------|--------------------------------------------------|
| `0x00000000 - 0x00001000` | Debug ROM                                     |
| `0x02000000 - 0x0200FFFF` | CLINT (Timer/Software Interrupts)             |
| `0x0C000000 - 0x0FFFFFFF` | PLIC (External Interrupts)                    |
| `0x10000000 - 0x100000FF` | UART 16550A                                   |
| `0x80000000 - 0x87FFFFFF` | **DRAM (128MB by default)**                   |

The kernel is loaded at `0x80000000`. Everything after the kernel's code and data sections in DRAM is available for the **Page Allocator**.

---

## 1. Physical Memory Management (PMM)

The PMM tracks which 4KB pages are free. The most efficient bare-metal structure is a **Free List**.

### The "Zero-Overhead" Free List
Since free pages are not being used, we store the "next" pointer of the linked list directly in the first 4 bytes of the free page itself.

- **`alloc_page()`**: Pop the head of the list.
- **`free_page(ptr)`**: Push the pointer back to the head of the list.

### Finding Available Memory
The kernel uses symbols exported by the linker script to determine where the "heap" begins:
- `_end`: The first byte after the kernel's BSS section.
- Memory from `_end` to `0x88000000` is managed by the PMM.

---

## 2. Virtual Memory Management (Sv32)

When paging is enabled, every instruction uses **Virtual Addresses (VA)**. The hardware translates these to **Physical Addresses (PA)** using the **satp** CSR and Page Tables.

### `satp` (Supervisor Address Translation and Protection) Register

This CSR tells the CPU where the root page table is and which mode to use.

```
 31   30         22 21                          0
┌────┬─────────────┬─────────────────────────────┐
│MODE│    ASID     │             PPN             │
└────┴─────────────┴─────────────────────────────┘
```

| Field  | Bits    | Description                                               |
|--------|---------|-----------------------------------------------------------|
| `MODE` | [31]    | `0` = Bare (No translation), `1` = Sv32 (Paging enabled) |
| `ASID` | [30:22] | Address Space ID (used to optimize TLB flushes)           |
| `PPN`  | [21:0]  | Physical Page Number of the root page table (`Addr >> 12`)|

---

## 3. Sv32 Page Table Structure

Sv32 uses a **2-level hierarchical page table**.

1.  **Level 1 (Root):** A 4KB page containing 1024 Page Table Entries (PTEs). Each entry maps a 4MB region (a "MegaPage").
2.  **Level 2 (Leaf):** A 4KB page containing 1024 PTEs. Each entry maps a 4KB region.

### Virtual Address Breakdown
A 32-bit virtual address is split into three parts:
- **VPN[1]** (10 bits): Index into the Level 1 table.
- **VPN[0]** (10 bits): Index into the Level 2 table.
- **Offset** (12 bits): Byte offset within the 4KB physical page.

### Page Table Entry (PTE) Format

Each 32-bit PTE looks like this:

```
 31                10 9   8 7 6 5 4 3 2 1 0
┌────────────────────┬───┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
│        PPN         │RSW│D│A│G│U│X│W│R│V│
└────────────────────┴───┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
```

| Bit | Name | Description                                      |
|-----|------|--------------------------------------------------|
| 0   | `V`  | Valid. If 0, any access triggers a Page Fault.   |
| 1   | `R`  | Readable.                                        |
| 2   | `W`  | Writable.                                        |
| 3   | `X`  | Executable.                                      |
| 4   | `U`  | User. If 1, accessible in U-mode.                |
| 5   | `G`  | Global. Mapping exists in all address spaces.    |
| 6   | `A`  | Accessed. Set by hardware when page is read.     |
| 7   | `D`  | Dirty. Set by hardware when page is written.     |
| 8-9 | `RSW`| Reserved for Software (OS use).                  |

**The "Leaf" Rule:**
- If `R=0, W=0, X=0`, the PTE points to the *next level* of the page table.
- If any of `R, W, X` are 1, it is a **Leaf PTE** pointing to a physical page.

---

## 4. The Translation Process (Walk)

When the CPU accesses address `VA`:
1.  Hardware finds the Root Table at `satp.PPN << 12`.
2.  It uses `VA.VPN[1]` to find the L1-PTE.
3.  If L1-PTE is not a leaf, it uses L1-PTE.PPN to find the L2 Table.
4.  It uses `VA.VPN[0]` to find the L2-PTE.
5.  Physical Address = `L2-PTE.PPN << 12 | VA.Offset`.

---

## 5. Key Concepts

### TLB (Translation Lookaside Buffer)
The CPU caches recent translations. If you change a page table, you **must** invalidate the cache using the `sfence.vma` instruction.

### Faults
If translation fails (e.g., `V=0` or permission violation), the hardware triggers:
- `Instruction Page Fault` (cause 12)
- `Load Page Fault` (cause 13)
- `Store/AMO Page Fault` (cause 15)

The faulting virtual address is stored in the `stval` (Supervisor) or `mtval` (Machine) register.
