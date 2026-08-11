;;;
;;; Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
;;;
;;; SPDX-License-Identifier: BSD-2-Clause
;;;

[bits 32]

#include <config.h>
#include "paging.h"

global  _start
extern  _start64

STACK_TOP equ 0x80000 ; NOTE: EBDA is here, temporary until _start64

;; FIXME:
NULL_SEL       equ 0x00
KERN_CODE_SEL  equ 0x08
KERN_DATA_SEL  equ 0x10
USER_DATA_SEL  equ 0x18 | 3 ; RPL=3
USER_CODE_SEL  equ 0x20 | 3 ; RPL=3
BOOT_CODE_SEL  equ 0x28

;;
;; We jump here after entering protected mode. First, we must reload our segment
;; registers. Note that CS has already been set due to the far jump performed to
;; get here. After this, we far jump to our kernel entry point.
;;
section .text.boot exec align=16
_start:
    mov   ax, KERN_DATA_SEL
    mov   ds, ax
    mov   es, ax
    mov   fs, ax
    mov   gs, ax
    mov   ss, ax
    mov   esp, STACK_TOP

    mov   eax, cr4
    or    eax, 0x20 ; CR4.PAE
    mov   cr4, eax

    ;; Clear page translation tables
    mov   edi, PML4T_PADDR
    mov   cr3, edi
    xor   eax, eax
    mov   ecx, PTT_SIZE
    rep   stosd

    ;; PML4[0] -> PDPT ;; WARN: Marked as user-accessible
    mov   edi, PML4T_PADDR
    mov   dword [edi], PDPT_PADDR | PTTE_US | PTTE_P | PTTE_RW

    ;; PDPT[0] -> PDT ;; WARN: Marked as user-accessible
    mov   edi, PDPT_PADDR
    mov   dword [edi], PDT_PADDR | PTTE_US | PTTE_P | PTTE_RW

    ;; PML4[511] -> PDPT_HI
    mov   edi, PML4T_PADDR + PML4_IDX(KERN_VMA) * PTTE_SIZE
    mov   dword [edi], PDPT_HI_PADDR | PTTE_P | PTTE_RW

    ;; PDPT_HI[510] -> PDT
    mov   edi, PDPT_HI_PADDR + PDPT_IDX(KERN_VMA) * PTTE_SIZE
    mov   dword [edi], PDT_PADDR | PTTE_P | PTTE_RW

    ;; PDT with first 1GB identity mapped with 2MB pages
    mov   edi, PDT_PADDR
    mov   ebx, PTTE_P | PTTE_RW | PTTE_PS
    mov   ecx, PTT_ENTS ; 512 entries = 1GB
  .identity_loop:
    mov   [edi], ebx
    add   ebx, 0x200000 ; Next 2MB phys frame
    add   edi, 8
    loop  .identity_loop

    mov   ecx, 0xC0000080
    rdmsr
    or    eax, 0x100 ; LME bit
    wrmsr

    mov   eax, cr0
    or    eax, 0x80000000 ; CR0.PG
    mov   cr0, eax

    jmp   KERN_CODE_SEL:_trampoline

[bits 64]
_trampoline:
    mov   rax, _start64
    jmp   rax
