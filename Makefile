#
# Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
#
# SPDX-License-Identifier: BSD-2-Clause
#

include config.mk

CC      := gcc
LD      := ld
NASM    := nasm
OBJCOPY := objcopy

CFLAGS := -Werror -Wextra -Wall -Wno-error=comment \
          -fno-stack-protector -ffreestanding -nostdlib \
          -fno-asynchronous-unwind-tables \
          -m64 -O0 -g -c

LDFLAGS   := -m elf_x86_64 -z noexecstack
NASMFLAGS := -f elf64

DEFS := -DKERN_OFFSET=$(KERN_OFFSET) \
        -DKERN_STACK_SIZE=$(KERN_STACK_SIZE) \
        -DUSER_OFFSET=$(USER_OFFSET) \
        -DUSER_SECTORS=$(USER_SECTORS) \
        -DUSER_LBA=$(USER_LBA)

sector_count = $(shell echo $$(( ($$(stat -c%s $(1)) + 511) / 512 )))

KERN_C   := $(wildcard $(KERN)/*.c)
KERN_ASM := $(wildcard $(KERN)/*.asm)
KERN_LD  := $(KERN)/$(KERN).ld
KERN_OBJ := $(patsubst %,$(BUILD)/%.o,$(basename $(KERN_ASM) $(KERN_C)))
KERN_ELF := $(BUILD)/$(KERN)/$(KERN).elf
KERN_BIN := $(BUILD)/$(KERN)/$(KERN).bin

USER_C   := $(wildcard $(USER)/*.c)
USER_ASM := $(wildcard $(USER)/*.asm)
USER_OBJ := $(patsubst %,$(BUILD)/%.o,$(basename $(USER_ASM) $(USER_C)))
USER_ELF := $(BUILD)/$(USER)/$(USER).elf
USER_BIN := $(BUILD)/$(USER)/$(USER).bin

MBR_BIN := $(BUILD)/$(BOOT)/mbr.bin
TARGET  := $(BUILD)/disk.img

.PHONY: all qemu debug clean

all: $(TARGET)

$(TARGET): $(MBR_BIN) $(KERN_BIN) $(USER_BIN)
	@mkdir -p $(@D)
	dd if=/dev/zero of=$@ bs=512 count=$(DISK_SECTORS) 2>/dev/null
	dd if=$(MBR_BIN)  of=$@ bs=512 count=1 conv=notrunc 2>/dev/null
	dd if=$(KERN_BIN) of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null
	dd if=$(USER_BIN) of=$@ bs=512 seek=$(USER_LBA) conv=notrunc 2>/dev/null

$(MBR_BIN): $(BOOT)/mbr.asm $(KERN_BIN)
	@mkdir -p $(@D)
	$(NASM) -f bin $(DEFS) \
		-DKERN_SECTORS=$(call sector_count,$(KERN_BIN)) \
		$< -o $@

$(KERN_ELF): $(KERN_OBJ) $(KERN_LD)
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) --defsym KERN_OFFSET=$(KERN_OFFSET) \
		-T $(KERN_LD) -o $@ $(KERN_OBJ)

# TODO: User linker script
$(USER_ELF): $(USER_OBJ)
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) -Ttext $(USER_OFFSET) -o $@ $(USER_OBJ)

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -O binary -R .bss -R .comment $< $@

$(BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(DEFS) -MMD -MP $< -o $@

$(BUILD)/%.o: %.asm
	@mkdir -p $(@D)
	$(NASM) $(NASMFLAGS) $(DEFS) $< -o $@

qemu: $(TARGET)
	qemu-system-x86_64 -s -S -drive file=$<,format=raw \
		-m 1G -no-reboot -nographic \
		-d cpu_reset,int -D $(BUILD)/qemu.log

debug: $(TARGET)
	gdb -x debug.gdb

clean:
	rm -rf $(BUILD)

-include $(KERN_OBJ:.o=.d) $(USER_OBJ:.o=.d)
