/*
 * Copyright (c) 2025, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "kern_serial.h"
#include "kern_io.h"
#include "types.h"

#define COM1 0x3F8

void serial_init(void) {
  io_outb(COM1 + 1, 0x00); // Disable all interrupts
  io_outb(COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
  io_outb(COM1 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
  io_outb(COM1 + 1, 0x00); //                  (hi byte)
  io_outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
  io_outb(COM1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
  io_outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void serial_putc(char c) {
  while (!(io_inb(COM1 + 5) & 0x20))
    ; // Wait for transmit empty
  io_outb(COM1, c);
}

void serial_puts(const char *str) {
  while (*str) {
    if (*str == '\n')
      serial_putc('\r');
    serial_putc(*str++);
  }
}

void serial_putu32(uint32_t val) {
  int i;
  uint8_t n;
  char str[11];

  str[0] = '0';
  str[1] = 'x';
  for (i = 7; i >= 0; i--, val >>= 4) {
    n = val & 0xF;
    str[i + 2] = n < 10 ? n + '0' : n + 'A' - 10;
  }
  str[10] = '\0';

  serial_puts(str);
}

void serial_putu64(uint64_t val) {
  int i;
  uint8_t n;
  char str[19];

  str[0] = '0';
  str[1] = 'x';
  for (i = 15; i >= 0; i--, val >>= 4) {
    n = val & 0xF;
    str[i + 2] = n < 10 ? n + '0' : n + 'A' - 10;
  }
  str[18] = '\0';

  serial_puts(str);
}
