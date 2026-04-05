/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "io.h"
#include "types.h"

#define ATA_IO 0x1F0
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_DATA 0xA1

void io_disable_pic(void) {
  io_outb(PIC_MASTER_DATA, 0xFF);
  io_outb(PIC_SLAVE_DATA, 0xFF);
}

void io_ata_pio_read(uint32_t lba, uint8_t sectors, uint32_t *buf) {
  uint32_t sector_idx;
  uint8_t dword_idx, status;

  while (io_inb(ATA_IO + 7) & 0x80)
    ; // Wait for drive to be ready

  io_outb(ATA_IO + 2, sectors);                     // Sector count
  io_outb(ATA_IO + 3, (uint8_t)lba);                // LBA low
  io_outb(ATA_IO + 4, (uint8_t)(lba >> 8));         // LBA mid
  io_outb(ATA_IO + 5, (uint8_t)(lba >> 16));        // LBA high
  io_outb(ATA_IO + 6, 0xE0 | ((lba >> 24) & 0x0F)); // Drive/head
  io_outb(ATA_IO + 7, 0x20);                        // READ SECTORS command

  for (sector_idx = 0; sector_idx < sectors; sector_idx++) {
    do {
      status = io_inb(ATA_IO + 7);
    } while ((status & 0x80) || !(status & 0x08)); // Wait for drive to be ready
    for (dword_idx = 0; dword_idx < 128;
         dword_idx++) { // Read 128 dwords (1 sector)
      buf[sector_idx * 128 + dword_idx] = io_inl(ATA_IO);
    }
  }
}
