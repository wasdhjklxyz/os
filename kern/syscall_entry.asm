;;;
;;; Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
;;;
;;; SPDX-License-Identifier: BSD-2-Clause
;;;

[bits 64]

global __syscall_entry
extern __syscall_dispatch

;;
;; SYSCALL entry point - called when userspace executes SYSCALL instruction.
;; Hardware state on entry:
;;  RCX = user RIP
;;  R11 = user RFLAGS
;;  RAX = syscall number
;;  RDI, RSI, RDX, R10, R8, R9 = syscall arguments
;;  RSP = user stack
;;  CS/SS = kernel selectors (from STAR MSR)
;;  CPL = 0
;;
align  16
__syscall_entry:
    swapgs            ; Swap to kern GS and switch to kern stack
    mov   [gs:0], rsp ; Save user RSP
    mov   rsp, [gs:8] ; Load kernel RSP

    ;; Save syscall arguments, callee-saved registers, and user context
    push  rcx ; User RIP
    push  r11 ; User RFLAGS
    push  rbx
    push  rbp
    push  r12
    push  r13
    push  r14
    push  r15
    push  rdi
    push  rsi
    push  rdx
    push  r10
    push  r8
    push  r9

    ;; Call C dispatcher - RSI, RDX, R8, R9 already correct
    mov   rdi, rax
    mov   rcx, r10
    push  rax
    call  __syscall_dispatch
    add   rsp, 8 ; Pop saved RAX (we have new return value)

    ;; Restore syscall arguments, callee-saved registers, and user context
    pop   r9
    pop   r8
    pop   r10
    pop   rdx
    pop   rsi
    pop   rdi
    pop   r15
    pop   r14
    pop   r13
    pop   r12
    pop   rbp
    pop   rbx
    pop   r11 ; User RFLAGS
    pop   rcx ; User RIP

    ;; Restore user RSP
    mov   rsp, [gs:0]
    swapgs

    ;; Return to userspace
    o64 sysret
