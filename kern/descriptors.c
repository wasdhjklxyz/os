/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "descriptors.h"
#include "types.h"

#define TSS_STACK_SIZE 0x1000 // 4KB
static uint8_t __tss_rsp0_stack[TSS_STACK_SIZE] __attribute__((aligned(16)));

#define TSS_BASE ((uint64_t)&__tss)
#define TSS_LIMIT (sizeof(__tss) - 1)
#define TSS_ACCESS_TYPE (0b10000000 | 0x9) // P=1, DPL=0, Type=TSS Avail
#define TSS_FLAGS 0                        // G=0, AVL=0
static struct {
  __RESERVED(4);
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  __RESERVED(8);
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  __RESERVED(10);
  uint16_t iopb_base; // IOPB ignored if set to >= TSS size
} __attribute__((packed, aligned(16))) __tss = {
    .rsp0 = (uint64_t)&__tss_rsp0_stack[TSS_STACK_SIZE],
    .iopb_base = (uint16_t)TSS_LIMIT,
};

struct __gdt_sys_desc {
  uint16_t lim0;
  uint16_t base0;
  uint8_t base1;
  uint8_t access_type;
  uint8_t flags_lim1;
  uint8_t base2;
  uint32_t base3;
  __RESERVED(4);
} __attribute__((packed));

struct __gdt_desc {
  __RESERVED(5);
  uint8_t access;
  uint8_t flags;
  __RESERVED(1);
} __attribute__((packed));

#define CODE_DESC_FLAGS (0b00100000)       // D=0, L=1
#define CODE_DESC_ACCESS_DPL0 (0b10011010) // P=1, DPL=0, S=1, E=1, R=1
#define CODE_DESC_ACCESS_DPL3 (0b11111010) // P=1, DPL=3, S=1, E=1, R=1
#define DATA_DESC_FLAGS (0b00000100)       // DB=1
#define DATA_DESC_ACCESS_DPL0 (0b10010010) // P=1, DPL=0, S=1, W=1
#define DATA_DESC_ACCESS_DPL3 (0b11110010) // P=1, DPL=3, S=1, W=1

static struct {
  __RESERVED(8); // Null descriptor
  struct __gdt_desc kcode;
  struct __gdt_desc kdata;
  struct __gdt_desc udata;
  struct __gdt_desc ucode;
  struct __gdt_sys_desc tss;
} __attribute__((packed, aligned(16))) __gdt = {
    .kcode = {.access = CODE_DESC_ACCESS_DPL0, .flags = CODE_DESC_FLAGS},
    .kdata = {.access = DATA_DESC_ACCESS_DPL0, .flags = DATA_DESC_FLAGS},
    .udata = {.access = DATA_DESC_ACCESS_DPL3, .flags = DATA_DESC_FLAGS},
    .ucode = {.access = CODE_DESC_ACCESS_DPL3, .flags = CODE_DESC_FLAGS},
    .tss =
        {
            .lim0 = (uint16_t)TSS_LIMIT,
            .access_type = TSS_ACCESS_TYPE,
            .flags_lim1 = TSS_FLAGS | ((TSS_LIMIT >> 16) & 0x00FF),
        },
};

void descriptors_init(void) {
  struct {
    uint16_t limit;
    uint64_t base;
  } __attribute__((packed)) gdtr = {
      .limit = sizeof(__gdt) - 1,
      .base = (uint64_t)&__gdt,
  };

  __gdt.tss.base0 = (uint16_t)TSS_BASE;
  __gdt.tss.base1 = (uint16_t)(TSS_BASE >> 16);
  __gdt.tss.base2 = (uint8_t)(TSS_BASE >> 24);
  __gdt.tss.base3 = (uint32_t)(TSS_BASE >> 32);

  asm("lgdt %0" : : "m"(gdtr) : "memory");
  asm("ltr %w0" : : "r"((uint16_t)GDT_TSS_SEL) : "memory");
}
