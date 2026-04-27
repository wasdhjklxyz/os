/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdarg.h>

#include "io.h"
#include "serial.h"

#define COM1 0x3F8

#define U8_HEX_STR_LEN 5
#define U16_HEX_STR_LEN 7
#define U32_HEX_STR_LEN 11
#define U64_HEX_STR_LEN 19

static void __putx(uint64_t val, size_t len) {
  int i;
  uint8_t n;
  char str[len];

  str[0] = '0';
  str[1] = 'x';
  for (i = len - 4; i >= 0; i--, val >>= 4) {
    n = val & 0xF;
    str[i + 2] = n < 10 ? n + '0' : n + 'A' - 10;
  }
  str[len - 1] = '\0';

  serial_puts(str);
}

void serial_init(void) {
  io_outb(COM1 + 1, 0x00); // Disable all interrupts
  io_outb(COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
  io_outb(COM1 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
  io_outb(COM1 + 1, 0x00); //                  (hi byte)
  io_outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
  io_outb(COM1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
  io_outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
  serial_putc('\n');
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

void serial_printf(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);

  while (*fmt) {
    if (*fmt == '%' && *(++fmt)) {
      switch (*fmt++) {
      case 'b':
        __putx(va_arg(ap, int), U8_HEX_STR_LEN);
        continue;
      case 'w':
        __putx(va_arg(ap, int), U16_HEX_STR_LEN);
        continue;
      case 'd':
        __putx(va_arg(ap, uint32_t), U32_HEX_STR_LEN);
        continue;
      case 'q':
        __putx(va_arg(ap, uint64_t), U64_HEX_STR_LEN);
        continue;
      }
    }

    if (*fmt == '\n')
      serial_putc('\r');
    serial_putc(*fmt++);
  }

  va_end(ap);
}
