/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef __DESCRIPTORS_H
#define __DESCRIPTORS_H

// TODO: Remove magic numbers
#define GDT_NULL_SEL 0x00
#define GDT_KERN_CODE_SEL 0x08
#define GDT_KERN_DATA_SEL 0x10
#define GDT_USER_DATA_SEL (0x18 | 3) // RPL=3
#define GDT_USER_CODE_SEL (0x20 | 3) // RPL=3
#define GDT_TSS_SEL 0x28

void descriptors_init(void);

#endif // __DESCRIPTORS_H
