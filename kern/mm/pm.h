/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef PM_H
#define PM_H

#include <stdint.h>

#define PM_NULL_FRAME ((uint64_t)-1)
#define PM_FRAME_OF(phys_addr) ((phys_addr) >> 12)

typedef uint8_t pm_bitmap_word_t;

struct pm_region {
  uint64_t base;
  uint64_t len;
} __attribute__((packed));

const struct pm_region *pm_init(void);
uint64_t pm_alloc_frame(void);
void pm_free_frame(uint64_t phys_addr);
void pm_update_ptr(void);

#endif // PM_H
