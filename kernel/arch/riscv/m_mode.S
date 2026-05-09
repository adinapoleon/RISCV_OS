.section .text

.globl machine_init
.balign 4
machine_init:
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
    li t0, 0xB1FF
    csrw medeleg, t0

    # Delegate supervisor software/timer/external interrupt causes.
    li t0, 0x222
    csrw mideleg, t0

    # Install the S-mode trap vector.
    la t0, supervisor_trap_entry
    csrw stvec, t0

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
    csrr a0, mcause
    csrr a1, mepc
    csrr a2, mtval
    li a3, 0
    call machine_trap_handler

1:
    j 1b
