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

#define PAGE_SIZE 0x1000UL // 4KiB
#define PAGE_ALIGN_UP(x) (((x) + (PAGE_SIZE) - 1) & ~((PAGE_SIZE) - 1))

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

typedef uint64_t size_t;

#endif // __TYPES_H
