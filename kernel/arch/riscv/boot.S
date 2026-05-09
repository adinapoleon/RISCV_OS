.section .text.init
.globl _start

_start:
    la sp, _kernel_stack_top

    call zero_bss
    call machine_init
    call enter_supervisor

1:
    j 1b

.section .text
.globl zero_bss
.balign 4
zero_bss:
    la t0, __bss_start
    la t1, __bss_end

1:
    bgeu t0, t1, 2f
    sb zero, 0(t0)
    addi t0, t0, 1
    j 1b

2:
    ret
