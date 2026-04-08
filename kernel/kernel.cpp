//to avoid name mangling
extern "C" void kernel_main() {

    // UART: Universal Asynchronous Receiver/Transmitter
    // initalize a pointer to the UART memory-mapped I/O address
    volatile char *uart = (volatile char *)0x10000000;

    // string that will be printed to the UART
    const char *msg = "Hello world from RISC-V OS!\n";

    // loop through each character in the string and write it to the UART
    for (int i = 0; msg[i]; i++) {
        uart[0] = msg[i];
    }

    while (1) {} //infinite loop to prevent the kernel from exiting
}