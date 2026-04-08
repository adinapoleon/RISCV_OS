CXX = riscv64-unknown-elf-g++
AS  = riscv64-unknown-elf-g++
LD  = riscv64-unknown-elf-g++

CFLAGS = -march=rv32im -mabi=ilp32 -ffreestanding -nostdlib -O0 -fno-exceptions -fno-rtti

TARGET = kernel.elf

all: $(TARGET)

$(TARGET):
	$(AS) -c kernel/start.S -o start.o
	$(CXX) $(CFLAGS) -c kernel/kernel.cpp -o kernel.o
	$(LD) -T kernel/linker.ld start.o kernel.o -o $(TARGET)

run: $(TARGET)
	qemu-system-riscv32 -machine virt -nographic -bios none -kernel $(TARGET)

clean:
	rm -f *.o $(TARGET)