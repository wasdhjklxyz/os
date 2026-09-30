/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef UTIL_H
#define UTIL_H

/* FIXME: Having a "util" file is usually bad sign */

#include <stdint.h>

#define RESERVED(n) char _reserved_##n[n]
#define STATIC_ASSERT(cond) _Static_assert(cond, "STATIC_ASSERT: " #cond)

#define PAGE_SIZE (UINT64_C(0x1000)) // 4KiB
#define PAGE_ALIGN_UP(x) (((x) + (PAGE_SIZE) - 1) & ~((PAGE_SIZE) - 1))
#define PAGE_ALIGN_DOWN(x) ((x) & ~((PAGE_SIZE) - 1))

#endif // UTIL_H
