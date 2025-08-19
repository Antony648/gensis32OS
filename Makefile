ASM=nasm
BUILD_DIR=build
SRC_DIR=src
FLAGS=  -g 	-ffreestanding	-falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce  -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
KERNEL_FILES= ./build/kernel/kernel.asm.o  ./build/kernel/kernel.o  ./build/kernel/idt/idt.o  ./build/kernel/essentials/essentials.o  ./build/kernel/idt/idt.asm.o ./build/kernel/isr/isr.asm.o ./build/kernel/io/io.asm.o ./build/kernel/isr/isr.o ./build/kernel/heap/heap.o ./build/kernel/heap/kheap.o ./build/kernel/paging/paging.asm.o ./build/kernel/paging/paging.o ./build/kernel/disk/disk.o ./build/kernel/disk/disk_stream.o ./build/kernel/string/string.o ./build/kernel/ctype/ctype.o ./build/kernel/heap/heap32.o ./build/kernel/heap/heap_cream.o
.PHONY: all bootloader kernel clean kernel_asm_o kernel_o isr_asm_o isr_o idt_asm_o idt_o essentials_o  paging_asm_o paging_o disk_o  disk_stream_o  string_o ctype_o heap32_o heap_cream_o
all:	kernel bootloader
	rm  $(BUILD_DIR)/final/os.bin
	dd if=$(BUILD_DIR)/bootloader/boot.bin >> $(BUILD_DIR)/final/os.bin 
	dd if=$(BUILD_DIR)/kernel/kernel.bin>> $(BUILD_DIR)/final/os.bin
	dd if=/dev/zero bs=1048576 count=16 >> $(BUILD_DIR)/final/os.bin
	sudo mount -t vfat $(BUILD_DIR)/final/os.bin ./mount/disk
	sudo cp	./test.txt ./mount/disk
	sudo umount ./mount/disk

kernel:	kernel_asm_o	kernel_o 	idt_asm_o 	idt_o  essentials_o  isr_asm_o  io_asm_o   isr_o     heap_o  kheap_o  paging_asm_o paging_o  disk_o disk_stream_o  string_o ctype_o heap32_o heap_cream_o
	i686-elf-ld  -g -relocatable $(KERNEL_FILES)  -o  $(BUILD_DIR)/kernel/kernelreloc.o
	i686-elf-gcc -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel/kernel.bin $(FLAGS) -ffreestanding -O0 -nostdlib  $(BUILD_DIR)/kernel/kernelreloc.o
		
kernel_asm_o: $(SRC_DIR)/kernel/kernel.asm
	$(ASM) -f elf -g $(SRC_DIR)/kernel/kernel.asm -o $(BUILD_DIR)/kernel/kernel.asm.o
	
idt_asm_o:  $(SRC_DIR)/kernel/idt/idt.asm
	$(ASM) -f elf -g $(SRC_DIR)/kernel/idt/idt.asm -o $(BUILD_DIR)/kernel/idt/idt.asm.o
	
isr_asm_o: $(SRC_DIR)/kernel/isr/isr.asm
	$(ASM) -f elf -g $(SRC_DIR)/kernel/isr/isr.asm -o $(BUILD_DIR)/kernel/isr/isr.asm.o
	
essentials_o:	$(SRC_DIR)/kernel/essentials/essentials.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/essentials/essentials.c  -o $(BUILD_DIR)/kernel/essentials/essentials.o
	
isr_o:	$(SRC_DIR)/kernel/isr/isr.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/isr/isr.c  -o $(BUILD_DIR)/kernel/isr/isr.o
	
heap_o:	$(SRC_DIR)/kernel/heap/heap.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/heap/heap.c  -o $(BUILD_DIR)/kernel/heap/heap.o

kheap_o:	$(SRC_DIR)/kernel/heap/kheap.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/heap/kheap.c  -o $(BUILD_DIR)/kernel/heap/kheap.o


idt_o: $(SRC_DIR)/kernel/idt/idt.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/idt/idt.c  -o $(BUILD_DIR)/kernel/idt/idt.o

io_asm_o: $(SRC_DIR)/kernel/io/io.asm
	$(ASM) -f elf -g $(SRC_DIR)/kernel/io/io.asm -o $(BUILD_DIR)/kernel/io/io.asm.o

kernel_o: $(SRC_DIR)/kernel/kernel.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/kernel.c  -o $(BUILD_DIR)/kernel/kernel.o

paging_o: $(SRC_DIR)/kernel/kernel.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/paging/paging.c  -o $(BUILD_DIR)/kernel/paging/paging.o
	
paging_asm_o: $(SRC_DIR)/kernel/io/io.asm
	$(ASM) -f elf -g $(SRC_DIR)/kernel/paging/enable_paging.asm -o $(BUILD_DIR)/kernel/paging/paging.asm.o

bootloader:$(SRC_DIR)/bootloader/boot.asm
	$(ASM) -f bin -g $(SRC_DIR)/bootloader/boot.asm -o $(BUILD_DIR)/bootloader/boot.bin

disk_o: $(SRC_DIR)/kernel/disk/disk.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/disk/disk.c  -o $(BUILD_DIR)/kernel/disk/disk.o
	
disk_stream_o: $(SRC_DIR)/kernel/disk/disk_stream.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/disk/disk_stream.c  -o $(BUILD_DIR)/kernel/disk/disk_stream.o
	
string_o: $(SRC_DIR)/kernel/string/string.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/string/string.c  -o $(BUILD_DIR)/kernel/string/string.o
	
ctype_o: $(SRC_DIR)/kernel/ctype/ctype.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/ctype/ctype.c  -o $(BUILD_DIR)/kernel/ctype/ctype.o
	
heap32_o: $(SRC_DIR)/kernel/heap/heap32.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/heap/heap32.c  -o $(BUILD_DIR)/kernel/heap/heap32.o

heap_cream_o: $(SRC_DIR)/kernel/heap/heap_cream.c
	i686-elf-gcc -I ./src	$(FLAGS) -c -std=gnu99 $(SRC_DIR)/kernel/heap/heap_cream.c  -o $(BUILD_DIR)/kernel/heap/heap_cream.o


clean:
	rm -rf $(BUILD_DIR)/bootloader/boot.bin 
	rm -rf $(BUILD_DIR)/kernel/isr/*
	rm -rf $(BUILD_DIR)/kernel/idt/*
	rm -rf $(BUILD_DIR)/kernel/essentials/*
	rm -rf $(BUILD_DIR)/kernel/io/*
	rm -rf $(BUILD_DIR)/kernel/heap/*
	rm -rf $(BUILD_DIR)/kernel/paging/*
	rm -rf $(BUILD_DIR)/kernel/disk/*
	rm -rf $(BUILD_DIR)/kernel/string/*
	rm -f  $(BUILD_DIR)/kernel/ctype/*
	find $(BUILD_DIR)/kernel/ -maxdepth 1 -type f -delete 
