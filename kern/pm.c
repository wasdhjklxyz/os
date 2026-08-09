#include "pm.h"
#include "serial.h"

// FIXME: This is also in boot/mbr.asm (must match) so refactor!
#define MMAP_ENT 0x5000
#define MMAP_ENT_START 0x5004

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

// NOTE: For use later when we update the available memory
// struct pm_region_node {
//   struct pm_region region;
//   struct pm_region_node *next;
// };
// struct pm_map {
//   size_t size;
//   struct pm_region_node *regions;
// };
static struct pm_region avail = {}; // TODO: Update to map "later"

int pm_init(void) {
  const uint32_t count = *(const uint32_t *)MMAP_ENT;
  const struct pm_ent *ents = (const struct pm_ent *)MMAP_ENT_START;

  /* WARN: There could be some fuckery here finding whats truly available. The
           reigion > 1MiB is not standardized, well-defined, or contiguous.
     NOTE: We completely ignore memory in the first 1MiB for practicality */
  for (uint32_t i = 0; i < count; i++) {
    if (ents[i].type == AVAILABLE && ents[i].region.base >= 0x100000) {
      avail.base = ents[i].region.base;
      avail.len = ents[i].region.len;
      break;
    };
  }

  if (avail.len == 0) {
    serial_printf("No available memory found!");
    return -1;
  }
  serial_printf("Available memory found:\n");
  serial_printf("  base=%q\n", avail.base);
  serial_printf("  len=%q\n", avail.len);

  return 0;
}
