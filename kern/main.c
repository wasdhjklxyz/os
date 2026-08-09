/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "descriptors.h"
#include "events.h"
#include "io.h"
#include "pm.h"
#include "serial.h"
#include "syscall.h"
#include "vm.h"

#ifndef USER_LBA
#define USER_LBA 0
#endif // USER_LBA
#ifndef USER_SECTORS
#define USER_SECTORS 32
#endif // USER_SECTORS
#ifndef USER_OFFSET
#define USER_OFFSET 0
#endif // USER_OFFSET

#define USER_STACK (USER_OFFSET + 0x100000)

static void __init(void) {
  serial_init();
  descriptors_init();
  io_disable_pic();
  syscall_init();
  io_ata_pio_read(USER_LBA, USER_SECTORS, (uint32_t *)USER_OFFSET);
  (void)pm_init();
  vm_init();
  events_init();
};

static void __enter_user_mode(void) {
  asm("movq %0, %%rax\n\t"
      "movw %%ax, %%ds\n\t"
      "movw %%ax, %%es\n\t"
      "movw %%ax, %%fs\n\t"
      "movw %%ax, %%gs\n\t"
      "pushq %0\n\t"
      "pushq %1\n\t"
      "pushq $0x202\n\t"
      "pushq %2\n\t"
      "pushq %3\n\t"
      "iretq"
      :
      : "r"((uint64_t)GDT_USER_DATA_SEL), "r"((uint64_t)USER_STACK),
        "r"((uint64_t)GDT_USER_CODE_SEL), "r"((uint64_t)USER_OFFSET)
      : "rax", "memory");
}

void kern_main(void) {
  __init();
  __enter_user_mode();
}
