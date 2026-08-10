/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <config.h>

#include "types.h" // IWYU pragma: keep
#include "vm.h"

#define PDT 0x3000
#define PTT_US 0x04
#define PDTE_USER (USER_OFFSET / 0x200000) // Each entry maps 2MB

void vm_init(void) {
  uint64_t *pdt = (uint64_t *)PDT;
  pdt[PDTE_USER] |= PTT_US;
}
