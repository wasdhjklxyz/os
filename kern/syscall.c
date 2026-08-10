/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <config.h>

#include "descriptors.h"
#include "serial.h"

#define MSR_EFER 0xC0000080
#define MSR_STAR 0xC0000081
#define MSR_LSTAR 0xC0000082
#define MSR_SFMASK 0xC0000084
#define MSR_GS_BASE 0xC0000101
#define MSR_KERN_GS_BASE 0xC0000102
#define EFER_SCE (1 << 0)  // Syscall extensions
#define SFMASK_IF (1 << 9) // Interrupts

static uint8_t __stack[KERN_STACK_SIZE] __attribute__((aligned(16)));

static struct {
  uint64_t user_rsp;
  uint64_t kern_rsp;
} __attribute__((aligned(16))) __swapgs_data;

static inline uint64_t __rdmsr(uint64_t msr) {
  uint32_t low, high;
  asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr) : "memory");
  return ((uint64_t)high << 32) | low;
}

static inline void __wrmsr(uint64_t msr, uint64_t val) {
  uint32_t low = val & 0xFFFFFFFF;
  uint32_t high = val >> 32;
  asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high) : "memory");
}

extern void __syscall_entry(void);

long __syscall_dispatch(long num, long arg1, long arg2, long arg3, long arg4,
                        long arg5) {
  (void)arg2;
  (void)arg3;
  (void)arg4;
  (void)arg5;
  serial_printf("syscall: num=%d arg1=%d\n", num, arg1);
  return num;
}

void syscall_init(void) {
  __swapgs_data.kern_rsp = (uint64_t)&__stack[KERN_STACK_SIZE];
  __swapgs_data.user_rsp = 0; // Will be set on syscall entry

  __wrmsr(MSR_GS_BASE, (uint64_t)&__swapgs_data);
  __wrmsr(MSR_KERN_GS_BASE, (uint64_t)&__swapgs_data);

  uint64_t efer = __rdmsr(MSR_EFER);
  efer |= EFER_SCE;
  __wrmsr(MSR_EFER, efer);

  uint64_t star = 0;
  star |= ((uint64_t)GDT_KERN_CODE_SEL << 32);
  star |= ((uint64_t)((GDT_USER_CODE_SEL & ~3) - 16) << 48); // ~3 to remove RPL
  __wrmsr(MSR_STAR, star);

  __wrmsr(MSR_LSTAR, (uint64_t)__syscall_entry);

  __wrmsr(MSR_SFMASK, SFMASK_IF);
}
