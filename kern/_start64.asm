;;;
;;; Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
;;;
;;; SPDX-License-Identifier: BSD-2-Clause
;;;

[bits 64]

#include <config.h>

global  _start64
extern  kern_main
extern  kern_stack
extern  __bss_start
extern  __bss_end

section .text
_start64:
    ;; Zero .bss while RSP still points at low address bootstrap stack
    cld
    mov   rdi, __bss_start
    mov   rcx, __bss_end
    sub   rcx, rdi
    xor   eax, eax
    rep   stosb

    mov   rsp, kern_stack + KERN_STACK_SIZE
    call  kern_main
  .hang:
    hlt
    jmp   .hang
