/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef IO_H
#define IO_H

#include <stdint.h>

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

void io_disable_pic(void);
void io_ata_pio_read(uint32_t lba, uint8_t sectors, uint32_t *buf);

#endif // IO_H
