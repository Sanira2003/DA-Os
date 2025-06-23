# Makefile for Dinith_OS

# Tools
NASM := nasm
GCC := gcc
LD := ld
DD := dd
QEMU := qemu-system-i386

# Files
BOOT_SRC := boot.asm
BOOT_BIN := boot.bin
KERNEL_SRC := kernel.c
KERNEL_OBJ := kernel.o
KERNEL_BIN := kernel.bin
LINKER_SCRIPT := link.ld
DISK_IMAGE := DA-Os.img

# Flags
NASM_FLAGS := -f bin
GCC_FLAGS := -m16 -ffreestanding -fno-PIE -nostartfiles -nostdlib
LD_FLAGS := -m elf_i386 -T $(LINKER_SCRIPT) --oformat=binary

.PHONY: all clean run

all: $(DISK_IMAGE)

$(BOOT_BIN): $(BOOT_SRC)
	$(NASM) $(NASM_FLAGS) $< -o $@

$(KERNEL_OBJ): $(KERNEL_SRC)
	$(GCC) $(GCC_FLAGS) -c $< -o $@

$(KERNEL_BIN): $(KERNEL_OBJ) $(LINKER_SCRIPT)
	$(LD) $(LD_FLAGS) $< -o $@

$(DISK_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	$(DD) if=/dev/zero of=$@ bs=512 count=2880
	$(DD) if=$(BOOT_BIN) of=$@ conv=notrunc
	$(DD) if=$(KERNEL_BIN) of=$@ bs=512 seek=1 conv=notrunc

run: $(DISK_IMAGE)
	$(QEMU) -fda $<

clean:
	rm -f $(BOOT_BIN) $(KERNEL_OBJ) $(KERNEL_BIN) $(DISK_IMAGE)