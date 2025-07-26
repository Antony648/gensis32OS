ASM=nasm
BUILD_DIR=build
SRC_DIR=src
FLAGS=  -g 	-ffreestanding	-falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce  -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
KERNEL_FILES= ./build/kernel/kernel.asm.o  ./build/kernel/kernel.o  ./build/kernel/idt.o  ./build/kernel/essentials.o  ./build/kernel/idt.asm.o
.PHONY: all bootloader kernel clean 
all:	kernel bootloader
	rm  $(BUILD_DIR)/final/os.bin
	dd if=$(BUILD_DIR)/bootloader/boot.bin >> $(BUILD_DIR)/final/os.bin 
	dd if=$(BUILD_DIR)/kernel/kernel.bin>> $(BUILD_DIR)/final/os.bin
	dd if=/dev/zero bs=512 count=100 >> $(BUILD_DIR)/final/os.bin

kernel:	kernel_asm_o	kernel_o 	idt_asm_o 	idt_o  essentials_o                             
	i686-elf-ld  -g -relocatable $(KERNEL_FILES)  -o  $(BUILD_DIR)/kernel/kernelreloc.o
	i686-elf-gcc -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel/kernel.bin $(FLAGS) -ffreestanding -O0 -nostdlib  $(BUILD_DIR)/kernel/kernelreloc.o
		
kernel_asm_o: $(SRC_DIR)/kernel/kernel.asm
	$(ASM) -f elf -g $(SRC_DIR)/kernel/kernel.asm -o $(BUILD_DIR)/kernel/kernel.asm.o
	
idt_asm_o:  $(SRC_DIR)/kernel/idt.asm
	$(ASM) -f elf -g $(SRC_DIR)/kernel/idt.asm -o $(BUILD_DIR)/kernel/idt.asm.o
	
essentials_o:	$(SRC_DIR)/kernel/essentials.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/essentials.c  -o $(BUILD_DIR)/kernel/essentials.o

idt_o: $(SRC_DIR)/kernel/idt.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/idt.c  -o $(BUILD_DIR)/kernel/idt.o

kernel_o: $(SRC_DIR)/kernel/kernel.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/kernel.c  -o $(BUILD_DIR)/kernel/kernel.o

bootloader:$(SRC_DIR)/bootloader/boot.asm
	$(ASM) -f bin -g $(SRC_DIR)/bootloader/boot.asm -o $(BUILD_DIR)/bootloader/boot.bin

clean:
	rm -rf $(BUILD_DIR)/bootloader/boot.bin 
	rm -rf $(BUILD_DIR)/kernel/*

