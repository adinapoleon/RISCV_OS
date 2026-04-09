# QEMU `virt` Machine UART Hardware Specification

## Overview

In the QEMU `virt` machine (ARM/AArch64 and RISC-V), the UART (Universal Asynchronous Receiver/Transmitter) is a **16550A-compatible** device. It is memory-mapped at base address **`0x10000000`** and serves as the primary console interface for the guest system.

## Memory Map

| Address      | Size   | Description                         |
|--------------|--------|-------------------------------------|
| `0x10000000` | 8 bytes| 16550A UART registers (8-bit wide)  |

Only the first 8 register offsets are implemented and used.

## Register Map (Offset from Base)

All registers are 8 bits wide. Access size must be 8-bit (byte).

| Offset | DLAB | Access | Register | Abbr. | Description                     |
|--------|------|--------|----------|------|---------------------------------|
| 0x00   | 0    | Write  | Transmitter Holding Buffer | THR   | Data to transmit                |
| 0x00   | 0    | Read   | Receiver Buffer Register   | RBR   | Received data                   |
| 0x00   | 1    | R/W    | Divisor Latch Low Byte     | DLL   | Baud rate divisor (low byte)    |
| 0x01   | 0    | R/W    | Interrupt Enable Register  | IER   | Enable/disable interrupts       |
| 0x01   | 1    | R/W    | Divisor Latch High Byte    | DLM   | Baud rate divisor (high byte)   |
| 0x02   | -    | Read   | Interrupt Identification   | IIR   | Pending interrupt type          |
| 0x02   | -    | Write  | FIFO Control Register      | FCR   | FIFO mode and reset             |
| 0x03   | -    | R/W    | Line Control Register      | LCR   | Data format (bits, parity, stop)|
| 0x04   | -    | R/W    | Modem Control Register     | MCR   | Modem control signals           |
| 0x05   | -    | Read   | Line Status Register       | LSR   | Transmit/receive status         |
| 0x06   | -    | Read   | Modem Status Register      | MSR   | Modem signal status             |
| 0x07   | -    | R/W    | Scratch Register           | SCR   | No function, free for software  |

## DLAB (Divisor Latch Access Bit)

The DLAB is bit 7 of the **Line Control Register (LCR)**.
- When DLAB = 1, offsets 0x00 and 0x01 access the baud rate divisor latches (DLL, DLM).
- When DLAB = 0, offsets 0x00 and 0x01 access THR/RBR and IER.

## Line Status Register (LSR) - Offset 0x05

| Bit | Mask  | Name                       | Meaning                              |
|-----|-------|----------------------------|--------------------------------------|
| 0   | 0x01  | Data Ready (DR)            | Received data available in RBR       |
| 1   | 0x02  | Overrun Error (OE)         | Data overrun occurred                |
| 2   | 0x04  | Parity Error (PE)          | Parity error in received data        |
| 3   | 0x08  | Framing Error (FE)         | Stop bit error                       |
| 4   | 0x10  | Break Interrupt (BI)       | Break condition detected             |
| 5   | 0x20  | Transmitter Holding Empty (THRE) | THR is empty, ready for new data |
| 6   | 0x40  | Transmitter Empty (TEMT)   | Both THR and transmit shift reg empty|
| 7   | 0x80  | FIFO Error                 | At least one error in RX FIFO        |

## FIFO Control Register (FCR) - Offset 0x02 (Write Only)

| Bit | Name                        | Meaning                              |
|-----|-----------------------------|--------------------------------------|
| 0   | FIFO Enable                 | 1 = Enable FIFOs, 0 = 16550 mode (1 byte) |
| 1   | Receiver FIFO Reset         | 1 = Clear receiver FIFO (self-clearing) |
| 2   | Transmitter FIFO Reset      | 1 = Clear transmitter FIFO (self-clearing) |
| 3-4 | DMA Mode Select             | Not used in QEMU virt                |
| 5-6 | Receiver FIFO Trigger Level | Interrupt trigger level (00=1, 01=4, 10=8, 11=14 bytes) |
| 7   | Reserved                    |                                      |

## Interrupt Enable Register (IER) - Offset 0x01 (when DLAB=0)

| Bit | Interrupt Source            |
|-----|-----------------------------|
| 0   | Received Data Available     |
| 1   | Transmitter Holding Empty   |
| 2   | Receiver Line Status        |
| 3   | Modem Status                |
| 4-7 | Reserved (0)                |

## Interrupt Identification Register (IIR) - Offset 0x02 (Read)

| Bit | Description                         |
|-----|-------------------------------------|
| 0   | 0 = Interrupt pending, 1 = No interrupt |
| 1-2 | Interrupt identification (priority) |
| 3-7 | Various status and FIFO information |

## Physical Characteristics

- **Memory-mapped** (not I/O port-mapped)
- **8-bit** register access only
- **16550A compatible** (includes FIFO support)
- Base address fixed at `0x10000000` across all virt platforms
- Used by QEMU for `-serial` and `-nographic` console redirection