/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef __PM_H
#define __PM_H

#include "types.h"

// FIXME: This is also in boot/mbr.asm (must match) so refactor!
#define MMAP_ENT 0x5000

struct pm_ent {
  uint64_t base;
  uint64_t len;
  uint32_t type;
  uint32_t ext_attrs; // NOTE: ACPI 3.0 extended attributes
};

struct pm_ents {
  uint32_t count;
  struct pm_ent *ent;
};

/* TODO: Function that parses/validates the entries that the bootloader filled.
         I avoided writing one for now since no dynamic allocator and we don't
         know the count until runtime. */

#endif // __PM_H
