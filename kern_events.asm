;;;
;;; Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
;;;
;;; SPDX-License-Identifier: BSD-2-Clause
;;;

[bits 64]

global __events_de_stub, __events_db_stub, __events_nmi_stub, __events_bp_stub
global __events_of_stub, __events_br_stub, __events_uh_stub, __events_nm_stub
global __events_df_stub, __events_ts_stub, __events_np_stub, __events_ss_stub
global __events_gp_stub, __events_pf_stub, __events_mf_stub, __events_ac_stub
global __events_mc_stub, __events_xf_stub

extern __events_handler

%macro M_EVENT_NOERR 1
    push  0  ; Dummy error
    push  %1 ; Vector num
    jmp   __events_common_stub
%endmacro

%macro M_EVENT_ERR 1
    push  %1 ; Vector num (err already on stack)
    jmp   __events_common_stub
%endmacro

;;
;; FIXME: Use common vec nums header
;;
__events_de_stub:  M_EVENT_NOERR 0  ; Divide by zero error
__events_db_stub:  M_EVENT_NOERR 1  ; Debug
__events_nmi_stub: M_EVENT_NOERR 2  ; Non maskable interrupt
__events_bp_stub:  M_EVENT_NOERR 3  ; Breakpoint
__events_of_stub:  M_EVENT_NOERR 4  ; Overflow
__events_br_stub:  M_EVENT_NOERR 5  ; Bound-Range
__events_uh_stub:  M_EVENT_NOERR 6  ; Invalid-Opcode
__events_nm_stub:  M_EVENT_NOERR 7  ; Device not available
__events_df_stub:  M_EVENT_NOERR 8  ; Double fault
__events_ts_stub:  M_EVENT_ERR   10 ; Invalid TSS
__events_np_stub:  M_EVENT_ERR   11 ; Segment not present
__events_ss_stub:  M_EVENT_ERR   12 ; Stack
__events_gp_stub:  M_EVENT_ERR   13 ; General protection
__events_pf_stub:  M_EVENT_ERR   14 ; Page fault
__events_mf_stub:  M_EVENT_ERR   16 ; x87 floating-point exception pending
__events_ac_stub:  M_EVENT_NOERR 17 ; Alignment check
__events_mc_stub:  M_EVENT_NOERR 18 ; Machine check
__events_xf_stub:  M_EVENT_NOERR 19 ; SIMD floating point

align  16
__events_common_stub:
    push rax
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    mov  rdi, [rsp+120] ; Vector (15 pushes * 8 = 120)
    mov  rsi, [rsp+128] ; Error code
    call __events_handler
    pop  r15
    pop  r14
    pop  r13
    pop  r12
    pop  rbp
    pop  rbx
    pop  r11
    pop  r10
    pop  r9
    pop  r8
    pop  rsi
    pop  rdi
    pop  rdx
    pop  rcx
    pop  rax
    add  rsp, 16
    iretq
