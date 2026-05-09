CXX = riscv64-unknown-elf-g++
AS  = riscv64-unknown-elf-g++
LD  = riscv64-unknown-elf-g++

# Common flags for 32-bit RISC-V
ARCH_FLAGS = -march=rv32im_zicsr -mabi=ilp32

# C++ flags: freestanding, no standard library, no exceptions/RTTI, include headers
CXXFLAGS = $(ARCH_FLAGS) -ffreestanding -nostdlib -fno-exceptions -fno-rtti -O0 -Wall -Wextra -Ikernel

# Linker flags: use our script, no standard files, specify 32-bit emulation
LDFLAGS = $(ARCH_FLAGS) -T scripts/linker.ld -nostdlib -nostartfiles -Wl,-melf32lriscv

TARGET = kernel.elf

OBJS = boot.o m_mode.o entry.o trap_entry.o uart.o trap.o mem.o pmm.o vmm.o kernel.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)

boot.o: kernel/arch/riscv/boot.S
	$(AS) $(ARCH_FLAGS) -c kernel/arch/riscv/boot.S -o boot.o

m_mode.o: kernel/arch/riscv/m_mode.S
	$(AS) $(ARCH_FLAGS) -c kernel/arch/riscv/m_mode.S -o m_mode.o

entry.o: kernel/arch/riscv/entry.S
	$(AS) $(ARCH_FLAGS) -c kernel/arch/riscv/entry.S -o entry.o

trap_entry.o: kernel/arch/riscv/trap_entry.S
	$(AS) $(ARCH_FLAGS) -c kernel/arch/riscv/trap_entry.S -o trap_entry.o

uart.o: kernel/drivers/uart.cpp kernel/drivers/uart.h
	$(CXX) $(CXXFLAGS) -c kernel/drivers/uart.cpp -o uart.o

trap.o: kernel/trap/trap.cpp kernel/trap/trap.h kernel/drivers/uart.h
	$(CXX) $(CXXFLAGS) -c kernel/trap/trap.cpp -o trap.o

mem.o: kernel/memory/mem.cpp kernel/memory/mem.h
	$(CXX) $(CXXFLAGS) -c kernel/memory/mem.cpp -o mem.o

pmm.o: kernel/memory/pmm.cpp kernel/memory/pmm.h kernel/memory/mem.h
	$(CXX) $(CXXFLAGS) -c kernel/memory/pmm.cpp -o pmm.o

vmm.o: kernel/memory/vmm.cpp kernel/memory/vmm.h kernel/memory/pmm.h
	$(CXX) $(CXXFLAGS) -c kernel/memory/vmm.cpp -o vmm.o

kernel.o: kernel/core/kernel.cpp kernel/drivers/uart.h kernel/memory/pmm.h kernel/memory/vmm.h
	$(CXX) $(CXXFLAGS) -c kernel/core/kernel.cpp -o kernel.o

run: $(TARGET)
	qemu-system-riscv32 -machine virt -nographic -bios none -kernel $(TARGET)

clean:
	rm -f *.o $(TARGET)
