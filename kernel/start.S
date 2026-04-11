.section .text.init      # Initialization code section
.globl _start           # entry point for linker (make _start global)

_start:                     # entry point label
    li sp, 0x84000000       # load immediate, stack pointer initialization  

    # installing trap vector
    la t0, trap_entry      # load address of trap vector into t0
    csrw mtvec, t0          # write t0 to stvec register (set trap vector base address)

    call kernel_main           # call the main kernel function

1:                          #label 1 
    j 1b                 # jump back to label 1 (infinite loop to prevent exit)

# trap entry point
.global trap_entry
.balign 4               #align to 4 bytes
trap_entry:
    # Allocate space for trap frame on stack (31 registers * 4 bytes each)
    addi sp, sp, -124     # adjust stack pointer to allocate space for trap frame

    # save x1 to x31 registers onto the stack
    sw x1, 0(sp)          # save x1 at offset 0
    sw x2, 4(sp)          # save x2 at offset 4
    sw x3, 8(sp)          # save x3 at offset 8
    sw x4, 12(sp)         # save x4 at offset 12
    sw x5, 16(sp)         # save x5 at offset 16
    sw x6, 20(sp)         # save x6 at offset 20
    sw x7, 24(sp)         # save x7 at offset 24
    sw x8, 28(sp)         # save x8 at offset 28
    sw x9, 32(sp)         # save x9 at offset 32
    sw x10, 36(sp)        # save x10 at offset 36
    sw x11, 40(sp)        # save x11 at offset 40
    sw x12, 44(sp)        # save x12 at offset 44
    sw x13, 48(sp)        # save x13 at offset 48
    sw x14, 52(sp)        # save x14 at offset 52
    sw x15, 56(sp)        # save x15 at offset 56
    sw x16, 60(sp)        # save x16 at offset 60
    sw x17, 64(sp)        # save x17 at offset 64
    sw x18, 68(sp)        # save x18 at offset 68
    sw x19, 72(sp)        # save x19 at offset 72
    sw x20, 76(sp)        # save x20 at offset 76
    sw x21, 80(sp)        # save x21 at offset 80
    sw x22, 84(sp)        # save x22 at offset 84
    sw x23, 88(sp)        # save x23 at offset 88
    sw x24, 92(sp)        # save x24 at offset 92
    sw x25, 96(sp)        # save x25 at offset 96
    sw x26, 100(sp)       # save x26 at offset 100
    sw x27, 104(sp)       # save x27 at offset 104
    sw x28, 108(sp)       # save x28 at offset 108
    sw x29, 112(sp)       # save x29 at offset 112
    sw x30, 116(sp)       # save x30 at offset 116
    sw x31, 120(sp)       # save x31 at offset 120

    # pass arguments to trap handler 
    csrr a0, mcause        # read mcause register into a0 (trap cause)
    csrr a1, mepc          # read mepc register into a1 (program counter at time of trap)
    csrr a2, mtval         # read mtval register into a2 (trap value)
    mv a3, sp              # move current stack pointer (trap frame) into a3

    call trap_handler         # call the trap handler function

    # restore x1 to x31 registers from the stack
    lw x1, 0(sp)          # restore x1 from offset 0
    lw x2, 4(sp)          # restore x2 from offset 4
    lw x3, 8(sp)          # restore x3 from offset 8
    lw x4, 12(sp)         # restore x4 from offset 12
    lw x5, 16(sp)         # restore x5 from offset 16
    lw x6, 20(sp)         # restore x6 from offset 20
    lw x7, 24(sp)         # restore x7 from offset 24
    lw x8, 28(sp)         # restore x8 from offset 28
    lw x9, 32(sp)         # restore x9 from offset 32
    lw x10, 36(sp)        # restore x10 from offset 36
    lw x11, 40(sp)        # restore x11 from offset 40
    lw x12, 44(sp)        # restore x12 from offset 44
    lw x13, 48(sp)        # restore x13 from offset 48
    lw x14, 52(sp)        # restore x14 from offset 52
    lw x15, 56(sp)        # restore x15 from offset 56
    lw x16, 60(sp)        # restore x16 from offset 60
    lw x17, 64(sp)        # restore x17 from offset 64
    lw x18, 68(sp)        # restore x18 from offset 68
    lw x19, 72(sp)        # restore x19 from offset 72
    lw x20, 76(sp)        # restore x20 from offset 76
    lw x21, 80(sp)        # restore x21 from offset 80
    lw x22, 84(sp)        # restore x22 from offset 84
    lw x23, 88(sp)        # restore x23 from offset 88
    lw x24, 92(sp)        # restore x24 from offset 92
    lw x25, 96(sp)        # restore x25 from offset 96
    lw x26, 100(sp)       # restore x26 from offset 100
    lw x27, 104(sp)       # restore x27 from offset 104
    lw x28, 108(sp)       # restore x28 from offset 108
    lw x29, 112(sp)       # restore x29 from offset 112
    lw x30, 116(sp)       # restore x30 from offset 116
    lw x31, 120(sp)       # restore x31 from offset 120

    addi sp, sp, 124      # adjust stack pointer back to original position
    mret                    # return from trap (restore program counter and status) 