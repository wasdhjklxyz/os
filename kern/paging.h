/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef __PAGING_H
#define __PAGING_H

#define PTT_SIZE 4096
#define PTT_ENTS 512

#define PTTE_SIZE 8
#define PTTE_P (1 << 0)
#define PTTE_RW (1 << 1)
#define PTTE_US (1 << 2)
#define PTTE_PS (1 << 7)
#define PTTE_NX (1UL << 63)

#define PML4_IDX(va) (((va) >> 39) & 0x1FF)
#define PDP_IDX(va) (((va) >> 30) & 0x1FF)
#define PD_IDX(va) (((va) >> 21) & 0x1FF)
#define PT_IDX(va) (((va) >> 12) & 0x1FF)
#define PP_OFF(va) ((va) & 0xFFF)

#endif // __PAGING_H
