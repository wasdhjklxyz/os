/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <config.h>

#include "descriptors.h"
#include "events.h"
#include "io.h"
#include "paging.h"
#include "pm.h"
#include "serial.h"
#include "syscall.h"
#include "vm.h"

uint8_t kern_stack[KERN_STACK_SIZE] __attribute__((aligned(16)));

static void __init(void) {
  serial_init();
  descriptors_init();
  io_disable_pic();
  syscall_init();

  const struct pm_region *physmap = pm_init();
  if (!physmap)
    return;
  if (vm_init(0, physmap->len) < 0)
    return;
  pm_update_ptr(); // FIXME: See fn definition

  events_init();
};

static int __enter_user_mode(void) {
  const uintptr_t va = 0;

  // FIXME: The flags => executable AND read/write. Need I say more?
  if (vm_map_range(va, USER_OFFSET,
                   PAGE_ALIGN_UP(USER_SECTORS * DISK_BLOCK_SIZE),
                   PTTE_RW | PTTE_US) < 0)
    return -1;
  io_ata_pio_read(USER_LBA, USER_SECTORS, (uint32_t *)va);

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
      : "r"((uint64_t)GDT_USER_DATA_SEL), "r"((uint64_t)(va + PAGE_SIZE)),
        "r"((uint64_t)GDT_USER_CODE_SEL), "r"((uint64_t)va)
      : "rax", "memory");

  return 0;
}

void kern_main(void) {
  __init();
  __enter_user_mode();
}
