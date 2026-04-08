.section .text.init      # Initialization code section
.globl _start           # entry point for linker (make _start global)

_start:                     # entry point label
    li sp, 0x84000000       # load immediate, stack pointer initialization   
    call kernel_main           # call the main kernel function

1:                          #label 1 
    j 1b                 # jump back to label 1 (infinite loop to prevent exit)