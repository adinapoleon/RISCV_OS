CXX = riscv64-unknown-elf-g++
AS  = riscv64-unknown-elf-g++
LD  = riscv64-unknown-elf-g++

# Common flags for 32-bit RISC-V
ARCH_FLAGS = -march=rv32im_zicsr -mabi=ilp32

# C++ flags: freestanding, no standard library, no exceptions/RTTI, include headers
CXXFLAGS = $(ARCH_FLAGS) -ffreestanding -nostdlib -fno-exceptions -fno-rtti -O0 -Wall -Wextra -Iinclude

# Linker flags: use our script, no standard files, specify 32-bit emulation
LDFLAGS = $(ARCH_FLAGS) -T scripts/linker.ld -nostdlib -nostartfiles -Wl,-melf32lriscv

TARGET = kernel.elf

OBJS = start.o uart.o trap.o mem.o pmm.o vmm.o kernel.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)

start.o: src/kernel/start.S
	$(AS) $(ARCH_FLAGS) -c src/kernel/start.S -o start.o

uart.o: src/drivers/uart.cpp include/drivers/uart.h
	$(CXX) $(CXXFLAGS) -c src/drivers/uart.cpp -o uart.o

trap.o: src/kernel/trap.cpp include/kernel/trap.h include/drivers/uart.h
	$(CXX) $(CXXFLAGS) -c src/kernel/trap.cpp -o trap.o

mem.o: src/kernel/mem.cpp include/kernel/mem.h
	$(CXX) $(CXXFLAGS) -c src/kernel/mem.cpp -o mem.o

pmm.o: src/kernel/pmm.cpp include/kernel/pmm.h include/kernel/mem.h
	$(CXX) $(CXXFLAGS) -c src/kernel/pmm.cpp -o pmm.o

vmm.o: src/kernel/vmm.cpp include/kernel/vmm.h include/kernel/pmm.h
	$(CXX) $(CXXFLAGS) -c src/kernel/vmm.cpp -o vmm.o

kernel.o: src/kernel/kernel.cpp include/drivers/uart.h include/kernel/pmm.h include/kernel/vmm.h
	$(CXX) $(CXXFLAGS) -c src/kernel/kernel.cpp -o kernel.o

run: $(TARGET)
	qemu-system-riscv32 -machine virt -nographic -bios none -kernel $(TARGET)

clean:
	rm -f *.o $(TARGET)
