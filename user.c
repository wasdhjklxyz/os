/*
 * Copyright (c) 2025, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

static inline long syscall1(long num, long arg1) {
  long ret;
  asm volatile("syscall"
               : "=a"(ret)
               : "a"(num), "D"(arg1)
               : "rcx", "r11", "memory");
  return ret;
}

void do_page_fault(void) {
  unsigned long *foo = (unsigned long *)0x1000;
  unsigned long bar = *foo;
  (void)bar;
}

int main(void) {
  long result = syscall1(0xDEAD, 0xBEEF);

  do_page_fault();

  (void)result;
}
