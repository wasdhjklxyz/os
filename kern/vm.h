/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef __VM_H
#define __VM_H

#include <config.h>

#include "types.h" // IWYU pragma: keep

static inline void *vm_ptov(uint64_t addr) {
  return (void *)(addr + PHYSMAP_BASE);
}

static inline uint64_t vm_vtop(void *addr) {
  return (uint64_t)addr - PHYSMAP_BASE;
}

int vm_init(uintptr_t physmap_pa, size_t physmap_len);
int vm_map(uintptr_t va, uintptr_t pa, uint64_t flags);
int vm_map_range(uintptr_t va, uintptr_t pa, size_t len, uint64_t flags);

#endif // __VM_H
