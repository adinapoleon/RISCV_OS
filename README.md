# RISC-V Simple OS

A small bare-metal RISC-V OS written in freestanding C++ for the QEMU `virt`
machine. The kernel currently targets RV32IM with CSR support, boots from
Machine mode, drops into Supervisor mode, and uses the 16550A UART for console
output.

## Current Status

The kernel can boot under `qemu-system-riscv32`, initialize basic machine and
supervisor trap state, run PMM smoke checks, allocate a zeroed Sv32 root page
table, and idle cleanly.

Completed so far:

- Reset entry, stack setup, `.bss` zeroing, and linker-defined RAM/kernel
  symbols.
- M-mode setup for `mtvec`, PMP, trap delegation, `stvec`, and `mret` into
  S-mode.
- Supervisor trap entry that saves general-purpose registers and dispatches to
  C++ trap handling.
- Polled QEMU `virt` UART driver for strings, signed integers, and hex output.
- Bitmap-based physical page allocator over linker-defined RAM bounds.
- PMM protection for allocator metadata pages, invalid frees, and double frees.
- PMM accounting for total, used, and free pages.
- Zeroed page allocation for page-table construction.
- Sv32 VMM scaffold that allocates the root page table.
- Local and GitHub Actions smoke-test path that boots the kernel in QEMU and
  checks PMM/VMM bring-up output.

## Project Layout

```text
kernel/
  arch/riscv/   # reset entry, M-mode setup, S-mode entry, trap stubs
  core/         # kernel_main and boot smoke checks
  drivers/      # UART driver
  memory/       # mem helpers, PMM, VMM/Sv32 scaffold
  trap/         # C++ trap dispatch and trap-frame definitions
notes/          # hardware and subsystem notes
scripts/
  linker.ld     # QEMU virt memory layout and kernel symbols
  smoke.sh      # QEMU smoke-test runner
.github/
  workflows/    # CI build and smoke test
```

## Build And Run

Dependencies:

- `riscv64-unknown-elf-g++`
- `qemu-system-riscv32`
- `make`

Build:

```bash
make
```

Run interactively:

```bash
make run
```

Run the smoke test used by CI:

```bash
make smoke
```

The smoke test expects QEMU to time out because the kernel reaches its idle
loop. It fails if any boot check prints `[FAIL]` or if the expected PMM/VMM
bring-up markers are missing.

## Continuous Integration

GitHub Actions runs on every push and pull request. The workflow installs the
RISC-V bare-metal toolchain and QEMU, builds the kernel, then runs:

```bash
make smoke
```

## Roadmap

Immediate next steps:

- Finish Sv32 VMM mapping helpers.
- Identity-map the kernel image and required MMIO regions.
- Enable paging with `satp` mode `Sv32` and `sfence.vma`.
- Add cleaner linker sections/permissions so the ELF no longer emits the RWX
  LOAD segment warning.
- Add CLINT timer setup and supervisor timer interrupt handling.
- Add an `ecall`/syscall path.

Later:

- Kernel heap allocation.
- Scheduler groundwork and context switching.
- User-mode entry and basic `write`/`exit` syscalls.
- Interrupt-driven UART and PLIC support.

## Notes

Detailed hardware and subsystem notes live in `notes/`:

- `notes/TRAP.md`
- `notes/UART.md`
- `notes/MEMORY.md`
- `notes/RISCV_ISA.md`
