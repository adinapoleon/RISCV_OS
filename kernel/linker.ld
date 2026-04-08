/* Linker script for RISC-V OS */

ENTRY(_start) 
/* Entry point for the linker */

/* Section definitions */
SECTIONS
{
    /* Set the entry point for the linker */
    . = 0x80000000;

    /* text section - executable code */
    .text : { *(.text.init) *(.text*) }

    /* read-only data section - read-only global variables */
    .rodata : { *(.rodata*) }

    /* data section - initialized global variables */
    .data : { *(.data*) }

    /* bss section - Block Started by Symbol,
     * used for uninitialized global variables
     */
    .bss : { *(.bss*) *(COMMON) }
}