/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <config.h>

#include "paging.h"
#include "types.h" // IWYU pragma: keep
#include "vm.h"

#define PDTE_USER (USER_OFFSET / 0x200000) // Each entry maps 2MB

void vm_init(void) {
  uint64_t *pdt = (uint64_t *)PDT_PADDR;
  pdt[PDTE_USER] |= PTTE_US;
}
