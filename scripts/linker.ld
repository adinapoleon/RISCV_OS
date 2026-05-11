/* Linker script for RISC-V OS */

ENTRY(_start) 
/* Entry point for the linker */

PROVIDE(_ram_start = 0x80000000);
PROVIDE(_ram_end = 0x88000000);

/* Section definitions */
SECTIONS
{
    /* Set the entry point for the linker */
    . = _ram_start;

    PROVIDE(_kernel_start = .);

    /* text section - executable code */
    .text : { *(.text.init) *(.text*) }

    /* read-only data section - read-only global variables */
    .rodata : { *(.rodata*) }

    /* data section - initialized global variables */
    .data : { *(.data*) }

    /* bss section - Block Started by Symbol,
     * used for uninitialized global variables
     */
    .bss : {
        PROVIDE(__bss_start = .);
        *(.bss*) *(COMMON)
        PROVIDE(__bss_end = .);
    }

    . = ALIGN(16);
    PROVIDE(_kernel_stack_bottom = .);
    . += 16K;
    PROVIDE(_kernel_stack_top = .);

    PROVIDE(_end = .); /* Define the _end symbol at the end of the bss section */
}
