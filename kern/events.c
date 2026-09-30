/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "serial.h"
#include "types.h"

#define IDT_SIZE_MIN 32
#define IDT_SIZE_MAX 256
#define IDT_SIZE IDT_SIZE_MIN

struct int_trap_gate {
  uint64_t off;
  // uint16_t sel;
  // uint8_t ist;
  // uint8_t dpl;
};
typedef struct int_trap_gate int_gate_t;
typedef struct int_trap_gate trap_gate_t;

struct int_trap_gate_desc {
  uint16_t off_0;
  uint16_t sel;
  uint8_t ist;
  uint8_t type_attrs;
  uint16_t off_1;
  uint32_t off_2;
  RESERVED(4);
};
typedef struct int_trap_gate_desc int_gate_desc_t;
typedef struct int_trap_gate_desc trap_gate_desc_t;

enum {
  VEC_DE = 0,  // Divide by zero error
  VEC_DB,      // Debug
  VEC_NMI,     // Non maskable interrupt
  VEC_BP,      // Breakpoint
  VEC_OF,      // Overflow
  VEC_BR,      // Bound-Range
  VEC_UH,      // Invalid-Opcode
  VEC_NM,      // Device not available
  VEC_DF,      // Double fault
  VEC_TS = 10, // Invalid TSS
  VEC_NP,      // Segment not present
  VEC_SS,      // Stack
  VEC_GP,      // General protection
  VEC_PF,      // Page fault
  VEC_MF = 16, // x87 floating-point exception pending
  VEC_AC,      // Alignment check
  VEC_MC,      // Machine check
  VEC_XF,      // SIMD floating point
};

extern void events_de_stub(void);
extern void events_db_stub(void);
extern void events_nmi_stub(void);
extern void events_bp_stub(void);
extern void events_of_stub(void);
extern void events_br_stub(void);
extern void events_uh_stub(void);
extern void events_nm_stub(void);
extern void events_df_stub(void);
extern void events_ts_stub(void);
extern void events_np_stub(void);
extern void events_ss_stub(void);
extern void events_gp_stub(void);
extern void events_pf_stub(void);
extern void events_mf_stub(void);
extern void events_ac_stub(void);
extern void events_mc_stub(void);
extern void events_xf_stub(void);

STATIC_ASSERT(IDT_SIZE >= IDT_SIZE_MIN && IDT_SIZE <= IDT_SIZE_MAX);
static struct {
  struct int_trap_gate_desc table[IDT_SIZE];
  struct int_trap_gate gates[IDT_SIZE];
} idt = {
    .table = {{0}},
    .gates =
        {
            [VEC_DE] = {.off = (uint64_t)events_de_stub},
            [VEC_DB] = {.off = (uint64_t)events_db_stub},
            [VEC_NMI] = {.off = (uint64_t)events_nmi_stub},
            [VEC_BP] = {.off = (uint64_t)events_bp_stub},
            [VEC_OF] = {.off = (uint64_t)events_of_stub},
            [VEC_BR] = {.off = (uint64_t)events_br_stub},
            [VEC_UH] = {.off = (uint64_t)events_uh_stub},
            [VEC_NM] = {.off = (uint64_t)events_nm_stub},
            [VEC_DF] = {.off = (uint64_t)events_df_stub},
            [VEC_TS] = {.off = (uint64_t)events_ts_stub},
            [VEC_NP] = {.off = (uint64_t)events_np_stub},
            [VEC_SS] = {.off = (uint64_t)events_ss_stub},
            [VEC_GP] = {.off = (uint64_t)events_gp_stub},
            [VEC_PF] = {.off = (uint64_t)events_pf_stub},
            [VEC_MF] = {.off = (uint64_t)events_mf_stub},
            [VEC_AC] = {.off = (uint64_t)events_ac_stub},
            [VEC_MC] = {.off = (uint64_t)events_mc_stub},
            [VEC_XF] = {.off = (uint64_t)events_xf_stub},
        },
};

void events_handler(long vec, long err) {
  serial_printf("exception: v=%b e=%w\n", vec, err);
  asm("hlt");
}

void events_init(void) {
  struct int_trap_gate_desc *desc = NULL;
  struct int_trap_gate *gate = NULL;
  struct {
    uint16_t limit;
    uint64_t base;
  } __attribute__((packed)) idtr = {
      .limit = sizeof(idt.table) - 1,
      .base = (uint64_t)idt.table,
  };

  for (size_t i = 0; i < IDT_SIZE; i++) {
    gate = &idt.gates[i];
    if (gate->off == 0)
      continue;

    desc = &idt.table[i];
    desc->off_0 = (uint16_t)gate->off;
    desc->off_1 = (uint16_t)(gate->off >> 16);
    desc->off_2 = (uint32_t)(gate->off >> 32);
    desc->ist = 0;           // TODO: Differing IST idx? 0=Use TSS RSP
    desc->sel = 0x08;        // FIXME: Remove magic (KERN_CODE_SEL)
    desc->type_attrs = 0x8E; // FIXME: Remove magic (P=1, DPL=0, 64-bit)
  }

  asm("lidt %0" : : "m"(idtr));
}
