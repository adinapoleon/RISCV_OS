.section .text

.globl supervisor_main
.balign 4
supervisor_main:
    call kernel_main

1:
    j 1b
