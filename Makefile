#
# Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
#
# SPDX-License-Identifier: BSD-2-Clause
#

CC      := gcc
LD      := ld
NASM    := nasm
OBJCOPY := objcopy

BOOT  := boot
KERN  := kern
USER  := user
BUILD := build

CONFIG    := config
CONFIG_MK := $(BUILD)/config.mk
CONFIG_H  := $(BUILD)/include/config.h

CFLAGS := -Werror -Wextra -Wall -Wno-error=comment \
          -fno-stack-protector -ffreestanding -nostdlib \
          -fno-asynchronous-unwind-tables \
          -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone \
          -m64 -O0 -g -c

CPPFLAGS := -Iinclude -I$(BUILD)/include
ASPP := $(CC) -E -P -x assembler-with-cpp $(CPPFLAGS) -MMD -MP

LDFLAGS   := -m elf_x86_64 -z noexecstack
NASMFLAGS := -f elf64

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
.DEFAULT_GOAL := all

$(CONFIG_MK): $(CONFIG)
	@mkdir -p $(@D)
	sed -n 's/^\([A-Z_][A-Z0-9_]*\) *= *\([^ ]*\).*/\1 := \2/p' $< > $@

$(CONFIG_H): $(CONFIG)
	@mkdir -p $(@D)
	{ echo '#ifndef __CONFIG_H'; echo '#define __CONFIG_H'; \
	  sed -n 's/^\([A-Z_][A-Z0-9_]*\) *= *\([^ ]*\).*/#define \1 \2/p' $<; \
	  echo '#endif'; } > $@

ifeq (,$(filter clean,$(MAKECMDGOALS)))
include $(CONFIG_MK)
endif

all: $(TARGET)

$(TARGET): $(MBR_BIN) $(KERN_BIN) $(USER_BIN)
	@mkdir -p $(@D)
	dd if=/dev/zero of=$@ bs=512 count=$(DISK_SECTORS) 2>/dev/null
	dd if=$(MBR_BIN)  of=$@ bs=512 count=1 conv=notrunc 2>/dev/null
	dd if=$(KERN_BIN) of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null
	dd if=$(USER_BIN) of=$@ bs=512 seek=$(USER_LBA) conv=notrunc 2>/dev/null

$(KERN_OBJ) $(USER_OBJ) $(MBR_BIN): $(CONFIG_H)

$(MBR_BIN): $(BOOT)/mbr.asm $(KERN_BIN)
	@mkdir -p $(@D)
	$(ASPP) -DKERN_SECTORS=$(call sector_count,$(KERN_BIN)) $< -o $(@:.bin=.i)
	$(NASM) -f bin $(@:.bin=.i) -o $@

$(KERN_ELF): $(KERN_OBJ) $(KERN_LD)
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) \
		--defsym KERN_OFFSET=$(KERN_OFFSET) --defsym KERN_VMA=$(KERN_VMA) \
		-T $(KERN_LD) -o $@ $(KERN_OBJ)

# TODO: User linker script
$(USER_ELF): $(USER_OBJ)
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) -Ttext $(USER_OFFSET) -o $@ $(USER_OBJ)

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -O binary -R .bss -R .comment $< $@

$(BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP $< -o $@

$(BUILD)/%.o: %.asm
	@mkdir -p $(@D)
	$(ASPP) -MT $@ -MF $(@:.o=.d) $< -o $(@:.o=.i)
	$(NASM) $(NASMFLAGS) $(@:.o=.i) -o $@

qemu: $(TARGET)
	qemu-system-x86_64 -s -S -drive file=$<,format=raw \
		-m $(QEMU_MEM) -no-reboot -nographic \
		-d cpu_reset,int -D $(BUILD)/qemu.log

debug: $(TARGET)
	gdb -x debug.gdb

clean:
	rm -rf $(BUILD)

-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
