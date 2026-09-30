;;;
;;; Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
;;;
;;; SPDX-License-Identifier: BSD-2-Clause
;;;

[bits 64]

global events_de_stub, events_db_stub, events_nmi_stub, events_bp_stub
global events_of_stub, events_br_stub, events_uh_stub, events_nm_stub
global events_df_stub, events_ts_stub, events_np_stub, events_ss_stub
global events_gp_stub, events_pf_stub, events_mf_stub, events_ac_stub
global events_mc_stub, events_xf_stub

extern events_handler

%macro M_EVENT_NOERR 1
    push  0  ; Dummy error
    push  %1 ; Vector num
    jmp   events_common_stub
%endmacro

%macro M_EVENT_ERR 1
    push  %1 ; Vector num (err already on stack)
    jmp   events_common_stub
%endmacro

;;
;; FIXME: Use common vec nums header
;;
events_de_stub:  M_EVENT_NOERR 0  ; Divide by zero error
events_db_stub:  M_EVENT_NOERR 1  ; Debug
events_nmi_stub: M_EVENT_NOERR 2  ; Non maskable interrupt
events_bp_stub:  M_EVENT_NOERR 3  ; Breakpoint
events_of_stub:  M_EVENT_NOERR 4  ; Overflow
events_br_stub:  M_EVENT_NOERR 5  ; Bound-Range
events_uh_stub:  M_EVENT_NOERR 6  ; Invalid-Opcode
events_nm_stub:  M_EVENT_NOERR 7  ; Device not available
events_df_stub:  M_EVENT_NOERR 8  ; Double fault
events_ts_stub:  M_EVENT_ERR   10 ; Invalid TSS
events_np_stub:  M_EVENT_ERR   11 ; Segment not present
events_ss_stub:  M_EVENT_ERR   12 ; Stack
events_gp_stub:  M_EVENT_ERR   13 ; General protection
events_pf_stub:  M_EVENT_ERR   14 ; Page fault
events_mf_stub:  M_EVENT_ERR   16 ; x87 floating-point exception pending
events_ac_stub:  M_EVENT_NOERR 17 ; Alignment check
events_mc_stub:  M_EVENT_NOERR 18 ; Machine check
events_xf_stub:  M_EVENT_NOERR 19 ; SIMD floating point

align  16
events_common_stub:
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
    call events_handler
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
