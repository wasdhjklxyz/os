/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef __TYPES_H
#define __TYPES_H

#define NULL 0

#define __RESERVED(n) char _reserved_##n[n]
#define STATIC_ASSERT(cond) _Static_assert(cond, "STATIC_ASSERT: " #cond)

#define UINT64_C(c) (c##UL)
#define UINT64_MAX (UINT64_C(18446744073709551615))

#define PAGE_SIZE (UINT64_C(0x1000)) // 4KiB
#define PAGE_ALIGN_UP(x) (((x) + (PAGE_SIZE) - 1) & ~((PAGE_SIZE) - 1))
#define PAGE_ALIGN_DOWN(x) ((x) & ~((PAGE_SIZE) - 1))

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef unsigned long uintptr_t;
typedef unsigned long size_t;

#endif // __TYPES_H
