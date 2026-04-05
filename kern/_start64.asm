;;;
;;; Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
;;;
;;; SPDX-License-Identifier: BSD-2-Clause
;;;

[bits 64]

global  _start64
extern  kern_main

STACK_TOP equ 0x200000 ; 2MB

section .text
_start64:
    mov   rsp, STACK_TOP
    call  kern_main
  .hang:
    hlt
    jmp   .hang
