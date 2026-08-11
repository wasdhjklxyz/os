/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <config.h>

#include "descriptors.h"
#include "events.h"
#include "io.h"
#include "pm.h"
#include "serial.h"
#include "syscall.h"
#include "vm.h"

#define USER_STACK (USER_OFFSET + 0x100000) // FIXME: Remove this bs

uint8_t kern_stack[KERN_STACK_SIZE] __attribute__((aligned(16)));

static void __init(void) {
  serial_init();
  descriptors_init();
  io_disable_pic();
  syscall_init();
  io_ata_pio_read(USER_LBA, USER_SECTORS, (uint32_t *)USER_OFFSET);

  const struct pm_region *physmap = pm_init();
  if (!physmap)
    return;
  if (vm_init(physmap->base, physmap->len) < 0)
    return;

  events_init();
};

// static void __enter_user_mode(void) {
//   asm("movq %0, %%rax\n\t"
//       "movw %%ax, %%ds\n\t"
//       "movw %%ax, %%es\n\t"
//       "movw %%ax, %%fs\n\t"
//       "movw %%ax, %%gs\n\t"
//       "pushq %0\n\t"
//       "pushq %1\n\t"
//       "pushq $0x202\n\t"
//       "pushq %2\n\t"
//       "pushq %3\n\t"
//       "iretq"
//       :
//       : "r"((uint64_t)GDT_USER_DATA_SEL), "r"((uint64_t)USER_STACK),
//         "r"((uint64_t)GDT_USER_CODE_SEL), "r"((uint64_t)USER_OFFSET)
//       : "rax", "memory");
// }

void kern_main(void) {
  __init();
  // __enter_user_mode();
}
