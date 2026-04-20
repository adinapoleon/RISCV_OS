CXX = riscv64-unknown-elf-g++
AS  = riscv64-unknown-elf-g++
LD  = riscv64-unknown-elf-g++

# Common flags for 32-bit RISC-V
ARCH_FLAGS = -march=rv32im_zicsr -mabi=ilp32

# C++ flags: freestanding, no standard library, no exceptions/RTTI
CXXFLAGS = $(ARCH_FLAGS) -ffreestanding -nostdlib -fno-exceptions -fno-rtti -O0 -Wall -Wextra

# Linker flags: use our script, no standard files, specify 32-bit emulation
LDFLAGS = $(ARCH_FLAGS) -T kernel/linker.ld -nostdlib -nostartfiles -Wl,-melf32lriscv

TARGET = kernel.elf

all: $(TARGET)

$(TARGET): start.o uart.o trap.o pmm.o kernel.o
	$(LD) $(LDFLAGS) start.o uart.o trap.o pmm.o kernel.o -o $(TARGET)

start.o: kernel/start.S
	$(AS) $(ARCH_FLAGS) -c kernel/start.S -o start.o

uart.o: kernel/uart.cpp kernel/uart.h
	$(CXX) $(CXXFLAGS) -c kernel/uart.cpp -o uart.o

trap.o: kernel/trap.cpp kernel/trap.h
	$(CXX) $(CXXFLAGS) -c kernel/trap.cpp -o trap.o

pmm.o: kernel/pmm.cpp kernel/pmm.h
	$(CXX) $(CXXFLAGS) -c kernel/pmm.cpp -o pmm.o

kernel.o: kernel/kernel.cpp kernel/uart.h kernel/pmm.h
	$(CXX) $(CXXFLAGS) -c kernel/kernel.cpp -o kernel.o

run: $(TARGET)
	qemu-system-riscv32 -machine virt -nographic -bios none -kernel $(TARGET)

clean:
	rm -f *.o $(TARGET)