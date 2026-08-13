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

extern char __text_start[], __text_end[];
extern char __rodata_start[], __rodata_end[];
extern char __data_start[], __bss_end[];

static uintptr_t _kern_v2p(char *va) { return (uintptr_t)va - KERN_VMA; }
static uintptr_t _seclen(char *a, char *b) { return (size_t)(b - a); }

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

static uintptr_t table_base = 0;

static uintptr_t *_table_ptr(uintptr_t pa) {
  return (uintptr_t *)(pa + table_base);
}

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

static inline void _load_cr3(uintptr_t pa) {
  asm volatile("mov %0, %%cr3" : : "r"(pa) : "memory");
}

int vm_init(uintptr_t physmap_pa, size_t physmap_len) {
  if (vm_map_range(PHYSMAP_BASE, physmap_pa, physmap_len, 0) < 0)
    return -1;
  if (vm_map_range((uintptr_t)__text_start, _kern_v2p(__text_start),
                   _seclen(__text_start, __text_end), 0) < 0)
    return -1;
  if (vm_map_range((uintptr_t)__rodata_start, _kern_v2p(__rodata_start),
                   _seclen(__rodata_start, __rodata_end), 0) < 0)
    return -1;
  if (vm_map_range((uintptr_t)__data_start, _kern_v2p(__data_start),
                   _seclen(__data_start, __bss_end), 0) < 0)
    return -1;

  table_base = PHYSMAP_BASE;
  _load_cr3(_kern_v2p((char *)pml4));
  return 0;
}

int vm_map(uintptr_t va, uintptr_t pa, uint64_t flags) {
  uintptr_t *pml4e = &pml4[PML4_IDX(va)];
  if (!(*pml4e & PTTE_P) && !_alloc_table(pml4e))
    return -1;

  uintptr_t *pdpe = &(_table_ptr(PTTE_ADDR(*pml4e)))[PDP_IDX(va)];
  if (!(*pdpe & PTTE_P) && !_alloc_table(pdpe))
    return -1;

  uintptr_t *pde = &(_table_ptr(PTTE_ADDR(*pdpe)))[PD_IDX(va)];
  if (!(*pde & PTTE_P) && !_alloc_table(pde))
    return -1;

  uintptr_t *pte = &(_table_ptr(PTTE_ADDR(*pde)))[PT_IDX(va)];
  if (!(*pte & PTTE_P))
    *pte = PTTE_ADDR(pa) | flags | PTTE_P;
  return 0;
}

int vm_map_range(uint64_t va, uint64_t pa, size_t len, uint64_t flags) {
  if ((va | pa | len) & (PAGE_SIZE - 1))
    return -1;
  for (size_t i = 0; i < len / PAGE_SIZE; i++)
    if (vm_map(va + i * PAGE_SIZE, pa + i * PAGE_SIZE, flags) < 0)
      return -1;
  return 0;
}
