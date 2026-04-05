include config.mk

CC      := gcc
LD      := ld
NASM    := nasm
OBJCOPY := objcopy

CFLAGS  := -Werror -Wextra -Wall -Wno-error=comment \
					 -fno-stack-protector -ffreestanding -nostdlib \
					 -m64 -O0 -g -c

NASMFLAGS := -f elf64

KERN_C   := $(wildcard $(KERN_DIR)/*.c)
KERN_ASM := $(wildcard $(KERN_DIR)/*.asm)
KERN_OBJ := $(KERN_ASM:.asm=.o) $(KERN_C:.c=.o)
KERN_OBJ := $(KERN_DIR)/_start.o $(KERN_DIR)/_start64.o \
            $(filter-out %/_start.o %/_start64.o, $(KERN_ASM:.asm=.o) $(KERN_C:.c=.o))

USER_C   := $(wildcard $(USER_DIR)/*.c)
USER_ASM := $(wildcard $(USER_DIR)/*.asm)
USER_OBJ := $(USER_ASM:.asm=.o) $(USER_C:.c=.o)

sector_count = $(shell echo $$(( ($$(stat -c%s $(1)) + 511) / 512 )))

.PHONY: all qemu clean

all: disk.img

$(BOOT_DIR)/mbr.bin: $(BOOT_DIR)/mbr.asm kern.bin user.bin
	$(NASM) -f bin \
		-DKERN_OFFSET=$(KERN_OFFSET) \
		-DKERN_SECTORS=$(call sector_count,kern.bin) \
		-DUSER_LBA=$(USER_LBA) \
		$< -o $@

kern.elf: $(KERN_OBJ)
	$(LD) -m elf_x86_64 -Ttext $(KERN_OFFSET) -o $@ $^

user.elf: $(USER_OBJ)
	$(LD) -m elf_x86_64 -Ttext $(USER_OFFSET) -o $@ $^

%.bin: %.elf
	$(OBJCOPY) -O binary -j .text -j .rodata -j .data $< $@

%.o: %.c
	$(CC) $(CFLAGS) -DKERN_OFFSET=$(KERN_OFFSET) \
		-DUSER_OFFSET=$(USER_OFFSET) \
		-DUSER_SECTORS=$(call sector_count,user.bin) \
		-DUSER_LBA=$(USER_LBA) $< -o $@

%.o: %.asm
	$(NASM) $(NASMFLAGS) \
		-DKERN_OFFSET=$(KERN_OFFSET) \
		-DUSER_OFFSET=$(USER_OFFSET) $< -o $@

disk.img: $(BOOT_DIR)/mbr.bin kern.bin user.bin
	dd if=/dev/zero of=$@ bs=512 count=2048 2>/dev/null
	dd if=$(BOOT_DIR)/mbr.bin of=$@ bs=512 count=1 conv=notrunc 2>/dev/null
	dd if=kern.bin of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null
	dd if=user.bin of=$@ bs=512 seek=$(USER_LBA) conv=notrunc 2>/dev/null

qemu: disk.img
	qemu-system-x86_64 -s -S -drive file=$<,format=raw \
		-m 1G -no-reboot -nographic \
		-d cpu_reset,int -D qemu.log

clean:
	rm -f $(KERN_OBJ) $(USER_OBJ) *.bin *.elf *.img *.log \
		$(BOOT_DIR)/*.bin
