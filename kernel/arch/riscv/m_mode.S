.section .text

.globl machine_init
.balign 4
machine_init:
    addi sp, sp, -16
    sw ra, 12(sp)

    # Keep an M-mode trap vector for traps not delegated to S-mode.
    la t0, machine_trap_entry
    csrw mtvec, t0

    # Allow S-mode to access the whole 32-bit physical address space.
    li t0, 0xFFFFFFFF
    srli t0, t0, 2
    csrw pmpaddr0, t0

    # PMP entry 0: TOR region, RWX permissions.
    li t0, 0x0F
    csrw pmpcfg0, t0

    # Delegate common synchronous exceptions to S-mode.
    li t0, 0xB3FF
    csrw medeleg, t0

    # Delegate supervisor software/timer/external interrupt causes.
    li t0, 0x222
    csrw mideleg, t0

    # Install the S-mode trap vector.
    la t0, supervisor_trap_entry
    csrw stvec, t0

    call machine_timer_init

    lw ra, 12(sp)
    addi sp, sp, 16
    ret

.globl enter_supervisor
.balign 4
enter_supervisor:
    la t0, supervisor_main
    csrw mepc, t0

    # Clear MPP, then set it to Supervisor mode for mret.
    csrr t0, mstatus
    li t1, 0x1800
    not t1, t1
    and t0, t0, t1

    li t1, 0x0800
    or t0, t0, t1
    csrw mstatus, t0

    mret

.globl machine_trap_entry
.balign 4
machine_trap_entry:
    addi sp, sp, -128

    sw x1, 0(sp)
    sw x5, 16(sp)
    addi t0, sp, 128
    sw t0, 4(sp)
    sw x3, 8(sp)
    sw x4, 12(sp)
    sw x6, 20(sp)
    sw x7, 24(sp)
    sw x8, 28(sp)
    sw x9, 32(sp)
    sw x10, 36(sp)
    sw x11, 40(sp)
    sw x12, 44(sp)
    sw x13, 48(sp)
    sw x14, 52(sp)
    sw x15, 56(sp)
    sw x16, 60(sp)
    sw x17, 64(sp)
    sw x18, 68(sp)
    sw x19, 72(sp)
    sw x20, 76(sp)
    sw x21, 80(sp)
    sw x22, 84(sp)
    sw x23, 88(sp)
    sw x24, 92(sp)
    sw x25, 96(sp)
    sw x26, 100(sp)
    sw x27, 104(sp)
    sw x28, 108(sp)
    sw x29, 112(sp)
    sw x30, 116(sp)
    sw x31, 120(sp)

    csrr a0, mcause
    csrr a1, mepc
    csrr a2, mtval
    mv a3, sp
    call machine_trap_handler

    lw x1, 0(sp)
    lw x3, 8(sp)
    lw x4, 12(sp)
    lw x5, 16(sp)
    lw x6, 20(sp)
    lw x7, 24(sp)
    lw x8, 28(sp)
    lw x9, 32(sp)
    lw x10, 36(sp)
    lw x11, 40(sp)
    lw x12, 44(sp)
    lw x13, 48(sp)
    lw x14, 52(sp)
    lw x15, 56(sp)
    lw x16, 60(sp)
    lw x17, 64(sp)
    lw x18, 68(sp)
    lw x19, 72(sp)
    lw x20, 76(sp)
    lw x21, 80(sp)
    lw x22, 84(sp)
    lw x23, 88(sp)
    lw x24, 92(sp)
    lw x25, 96(sp)
    lw x26, 100(sp)
    lw x27, 104(sp)
    lw x28, 108(sp)
    lw x29, 112(sp)
    lw x30, 116(sp)
    lw x31, 120(sp)

    lw x2, 4(sp)
    mret
