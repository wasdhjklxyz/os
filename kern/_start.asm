;;;
;;; Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
;;;
;;; SPDX-License-Identifier: BSD-2-Clause
;;;

[bits 32]

global  _start
extern  _start64

PML4T_ADDR equ 0x1000
PDPT_ADDR  equ 0x2000
PDT_ADDR   equ 0x3000

PTT_SIZE  equ 4096
PTT_ENTS  equ 512
PTTE_SIZE equ 8

PTT_P     equ 1
PTT_RW    equ 2
PTT_US    equ 4
PTT_PS    equ (1 << 7)

STACK_TOP equ 0x200000 ; 2MB

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
section .text.boot
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
    mov   edi, PML4T_ADDR
    mov   cr3, edi
    xor   eax, eax
    mov   ecx, 0x3000 / 4 ; 12KB (PML4 + PDPT + PDT) / 4 bytes
    rep   stosd

    ;; PML4[0] -> PDPT
    mov   edi, PML4T_ADDR
    mov   dword [edi], PDPT_ADDR | PTT_US | PTT_P | PTT_RW

    ;; PDPT[0] -> PDT
    mov   edi, PDPT_ADDR
    mov   dword [edi], PDT_ADDR | PTT_US | PTT_P | PTT_RW

    ;; PDT with first 1GB identity mapped with 2MB pages
    mov   edi, PDT_ADDR
    mov   ebx, PTT_P | PTT_RW | PTT_PS
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

    jmp   KERN_CODE_SEL:_start64
