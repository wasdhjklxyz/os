#include "pm.h"
#include "serial.h"
#include "types.h"

// FIXME: This is also in boot/mbr.asm (must match) so refactor!
#define MMAP_ENT 0x5000
#define MMAP_ENT_START 0x5004

#define BITMAP_BYTES(frames) (((frames) + 7) / 8)
#define BITMAP_BITS (8 * sizeof(bitmap_word_t))

typedef uint8_t bitmap_word_t;

typedef enum uint32_t {
  AVAILABLE = 1,
  RESERVED,
  ACPI_RECLAIMABLE,
  ACPI_NVS,
  BAD_MEMORY,
} pm_ent_type;

struct pm_region {
  uint64_t base;
  uint64_t len;
} __attribute__((packed));

struct pm_ent {
  struct pm_region region;
  pm_ent_type type;
  uint32_t ext_attrs; // NOTE: ACPI 3.0 extended attributes
} __attribute__((packed));

struct pm_bitmap {
  struct pm_region region;
  size_t bytes;
  bitmap_word_t *ptr;
  size_t next_hint;
};

static struct {
  struct pm_region region;
  size_t frames;
  struct pm_bitmap bitmap;
} avail = {};

static void _bitmap_clear(size_t i) {
  avail.bitmap.ptr[i / BITMAP_BITS] &= ~(1U << (i % BITMAP_BITS));
}

static void _bitmap_set(size_t i) {
  avail.bitmap.ptr[i / BITMAP_BITS] |= (1U << (i % BITMAP_BITS));
}

static int _bitmap_test(size_t i) {
  return avail.bitmap.ptr[i / BITMAP_BITS] & (1U << (i % BITMAP_BITS));
}

static void _zero_frame(uint64_t phys_addr) {
  // WARN: Valid only while identity mapped */
  uint64_t *ptr = (uint64_t *)phys_addr;
  for (size_t i = 0; i < PAGE_SIZE / sizeof(uint64_t); i++)
    ptr[i] = 0;
}

int pm_init(void) {
  const uint32_t count = *(const uint32_t *)MMAP_ENT;
  const struct pm_ent *ents = (const struct pm_ent *)MMAP_ENT_START;
  struct pm_bitmap *bmp = &avail.bitmap;

  /* WARN: There could be some fuckery here finding whats truly available. The
           reigion > 1MiB is not standardized, well-defined, or contiguous.
     NOTE: We completely ignore memory in the first 1MiB for practicality */
  for (uint32_t i = 0; i < count; i++) {
    if (ents[i].type == AVAILABLE && ents[i].region.base >= 0x100000) {
      avail.region.base = ents[i].region.base;
      avail.region.len = ents[i].region.len;
      avail.frames = avail.region.len / PAGE_SIZE;
      break;
    };
  }

  if (avail.frames == 0) {
    serial_printf("No available memory found!");
    return -1;
  }

  bmp->bytes = BITMAP_BYTES(avail.frames);
  bmp->region.len = PAGE_ALIGN_UP(bmp->bytes);
  bmp->region.base = avail.region.base;
  bmp->ptr = (bitmap_word_t *)bmp->region.base;

  for (size_t i = 0; i < avail.frames; i++)
    _bitmap_clear(i);
  for (size_t i = avail.frames; i < bmp->bytes * BITMAP_BITS; i++)
    _bitmap_set(i);
  bmp->next_hint = bmp->region.len / PAGE_SIZE;
  for (size_t i = 0; i < bmp->next_hint; i++)
    _bitmap_set(i);
  return 0;
}

uint64_t pm_alloc_frame(void) {
  for (size_t n = 0; n < avail.frames; n++) {
    size_t i = (avail.bitmap.next_hint + n) % avail.frames;
    if (!_bitmap_test(i)) {
      _bitmap_set(i);
      avail.bitmap.next_hint = i + 1;
      uint64_t addr = avail.region.base + i * PAGE_SIZE;
      _zero_frame(addr);
      return addr;
    }
  }
  return PM_NULL_FRAME;
}

void pm_free_frame(uint64_t phys_addr) {
  if (phys_addr < avail.region.base)
    return;
  size_t i = (phys_addr - avail.region.base) / PAGE_SIZE;
  if (i >= avail.frames)
    return;
  if (_bitmap_test(i))
    _bitmap_clear(i);
}
