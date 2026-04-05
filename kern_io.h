/*
 * Copyright (c) 2025, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef __IO_H
#define __IO_H

#include "types.h"

static inline void io_outb(uint16_t port, uint8_t val) {
  asm("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t io_inb(uint16_t port) {
  uint8_t ret;
  asm("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline uint32_t io_inl(uint16_t port) {
  uint32_t ret;
  asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

#endif // __IO_H
