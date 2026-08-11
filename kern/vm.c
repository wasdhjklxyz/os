/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <config.h>

#include "paging.h"
#include "pm.h"
#include "types.h" // IWYU pragma: keep
#include "vm.h"

#define PTTE_ADDR(ptte) ((ptte) & 0x000FFFFFFFFFF000UL)

// extern char __kern_lma[], __kern_end[];
// extern char __text_start[], __text_end[];
// extern char __rodata_start[], __rodata_end[];
// extern char __data_start[], __bss_end[];

// static struct vm_free_node {
//   struct vm_free_node *prev;
//   struct vm_free_node *next;
//   uintptr_t start;
//   size_t size;
// } free = {
//     .prev = NULL,
//     .next = NULL,
//     .start = KERN_HEAP_BASE,
//     .size = KERN_HEAP_LEN,
// };

static uintptr_t pml4[PTT_ENTS] __attribute__((aligned(PAGE_SIZE)));

static uintptr_t _alloc_table(uintptr_t *ptte) {
  if (!ptte || *ptte & PTTE_P)
    return 0;
  uintptr_t pa = pm_alloc_frame();
  if (pa == PM_NULL_FRAME)
    return 0;
  // WARN: US bit set
  *ptte = (pa & 0x000FFFFFFFFFF000UL) | PTTE_US | PTTE_RW | PTTE_P;
  return pa;
}

int vm_init(uintptr_t physmap_pa, size_t physmap_len) {
  if (vm_map_range(PHYSMAP_BASE, physmap_pa, PAGE_ALIGN_UP(physmap_len), 0) < 0)
    return -1;
  // TODO...
  return 0;
}

int vm_map(uintptr_t va, uintptr_t pa, uint64_t flags) {
  uintptr_t *pml4e = &pml4[PML4_IDX(va)];
  if (!(*pml4e & PTTE_P) && !_alloc_table(pml4e))
    return -1;

  uintptr_t *pdpe = &((uintptr_t *)PTTE_ADDR(*pml4e))[PDP_IDX(va)];
  if (!(*pdpe & PTTE_P) && !_alloc_table(pdpe))
    return -1;

  uintptr_t *pde = &((uintptr_t *)PTTE_ADDR(*pdpe))[PD_IDX(va)];
  if (!(*pde & PTTE_P) && !_alloc_table(pde))
    return -1;

  uintptr_t *pte = &((uintptr_t *)PTTE_ADDR(*pde))[PT_IDX(va)];
  if (!(*pte & PTTE_P))
    *pte = (pa & 0xFFFFFFFFFF000UL) | flags | PTTE_P;
  return 0;
}

int vm_map_range(uint64_t va, uint64_t pa, size_t n, uint64_t flags) {
  if (n % PAGE_SIZE != 0)
    return -1;
  for (size_t i = 0; i < n / PAGE_SIZE; i++)
    if (vm_map(va + i * PAGE_SIZE, pa + i * PAGE_SIZE, flags) < 0)
      return -1;
  return 0;
}
