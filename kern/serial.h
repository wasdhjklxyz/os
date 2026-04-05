/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef __SERIAL_H
#define __SERIAL_H

#include "types.h"

void serial_init(void);
void serial_putc(char c);
void serial_puts(const char *str);
void serial_printf(const char *fmt, ...);

#endif // __SERIAL_H
