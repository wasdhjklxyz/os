;;;
;;; Copyright (c) 2025, uiop <uiop@wasdhjkl.xyz>
;;;
;;; SPDX-License-Identifier: BSD-2-Clause
;;;

;;
;; CPU begins execution in 16-bit real mode with the BIOS loading us at 0x7C00.
;;
[bits 16]
[org  0x7C00]

#include <config.h>

;; FIXME:
NULL_SEL       equ 0x00
KERN_CODE_SEL  equ 0x08
KERN_DATA_SEL  equ 0x10
USER_DATA_SEL  equ 0x18 | 3 ; RPL=3
USER_CODE_SEL  equ 0x20 | 3 ; RPL=3
BOOT_CODE_SEL  equ 0x28

;;
;; This macro checks if the A20 line was enabled by attempting to write to two
;; addresses that would alias if A20 is disabled.
;;
;; It works by writing 0x00 to 0x1000, writing 0xFF to what should be 0x100FF0,
;; and then checking if 0x1000 was overwritten (indicating A20 line disabled).
;;
;; OUTPUT: ZF=1 (enabled), ZF=0 (disabled)
;; NOTE: Original values at test addrs are preserved
;;
%macro M_A20_CHECK 0
    push  ds
    push  es
    xor   ax, ax
    mov   ds, ax
    mov   si, 0x1000
    not   ax
    mov   es, ax
    mov   di, 0x1000         ; ES:DI = FFFF:1000 = 0x100FF0
    mov   al, byte [ds:si]   ; Save byte DS:SI
    mov   ah, byte [es:di]   ; Save byte ES:DI
    mov   byte [ds:si], 0x00 ; Set byte 0x1000 to 0x00
    mov   byte [es:di], 0xFF ; Set byte 0x100FF0 to 0xFF
    cmp   byte [ds:si], 0    ; Check if 0x1000 was overwritten
    mov   byte [es:di], ah   ; Restore ES:DI
    mov   byte [ds:si], al   ; Restore DS:SI
    pop   es
    pop   ds
%endmacro

;;
;; We do not know if the BIOS loaded us to 7C00:0000 or 0000:7C00. To
;; address this, we reload CS to 0x0000 by performing a far jump.
;;
;; After this we, zero out our GPRs and set our stack to start at 0x7C00.
;;
start:
    cli
    jmp   0x0000:.flush_cs
  .flush_cs:
    xor   ax, ax
    mov   ds, ax
    mov   es, ax
    mov   fs, ax
    mov   gs, ax
    mov   ss, ax
    mov   sp, 0x7C00
    sti

;;
;; Here we attempt to enable A20 line by trying BIOS INT 15h or via port 0x92.
;;
enable_a20_line:
    M_A20_CHECK
    jz    .done
    mov   ax, 0x2401 ; BIOS INT 15h enable A20 function
    int   0x15
    M_A20_CHECK
    jz    .done
    in    al, 0x92   ; Read byte from port 0x92
    or    al, 2      ; Enable bit 2
    out   0x92, al   ; Write the byte back
    M_A20_CHECK
    jz    .done
    jmp   error
  .done:

;;
;; Here we use BIOS INT 13h AH=41h to check if disk read extensions are present.
;;
check_disk_read_exts:
    mov   ah, 0x41
    mov   bx, 0x55AA ; Magic
    int   0x13
    jnc   load_kern
    jmp   error

;;
;; If extensions are installed, we can use BIOS interrupt call 13h AH=42h to
;; load the kernel. We do this before switching to protected mode since we'll
;; lose access to the BIOS ISRs.
;;
load_kern:
    mov   si, dap.kern_code
    mov   ah, 0x42
    mov   cx, 3
  .loop:
    int   0x13
    jnc   query_addr_map
    loop  .loop
    jmp   error

;;
;; Before entering protected mode we have to use another BIOS ISR to query the
;; system address map of all the installed RAM, and of physical memory ranges
;; reserved by the BIOS.
;;
query_addr_map:
    mov   eax, 0xE820
    xor   ebx, ebx              ; EBX must be 0 to start
    xor   bp, bp                ; Keep entry count in BP
    mov   edx, 0x0534D4150      ; "SMAP"
    mov   di, MMAP_ENT_START    ; Prevent getting stuck in INT 15h
    mov   ecx, 24               ; Ask for 24 bytes
    mov   [es:di + 20], dword 1 ; Force valid ACPI 3.X entry
    int   0x15
    jc    error
    mov   edx, 0x0534D4150      ; "SMAP" - some BIOSs trash this register
    cmp   eax, edx              ; EAX set to "SMAP" on success
    jne   error
    test  ebx, ebx              ; EBX=0 implies list only 1 entry (worthless)
    je    error
    jmp   .jmpin
  .e820lp:
    mov   eax, 0xE820           ; EAX gets trashed on every INT 15h call
    mov   [es:di + 20], dword 1 ; Force valid ACPI 3.X entry
    mov   ecx, 24               ; Ask for 24 bytes (again)
    int   0x15
    jc    .e820f                ; End of list already reached
    mov   edx, 0x0534D4150      ; "SMAP" - some BIOSs trash this register
  .jmpin:
    jcxz  .skipent
    cmp   cl, 20                ; Got a 24 byte ACPI 3.X response?
    jbe   .notext
  .notext:
    mov   ecx, [es:di + 8]      ; Get lower uint32_t of memory region length
    or    ecx, [es:di + 12]     ; OR it with upper uint32_t to test for zero
    jz    .skipent              ; If length uint64_t is 0, skip entry
    inc   bp                    ; Got good entry, increase count
    add   di, 24                ; Move to next storage spot
    cmp   di, MMAP_ENT_END - 24 ; Room for more?
    ja    .e820f                ; Out of space, stop early
  .skipent:
    test  ebx, ebx              ; If EBX resets to 0, list is complete
    jne   .e820lp
  .e820f:
    mov   [es:MMAP_ENT], bp     ; Store entry count
    clc                         ; There is "jc" on end of list so clear carry

;;
;; Now that we have taken advantage of the BIOS ISRs, we enter protected mode.
;; To do this, we disable interrupts, load GDTR & IDTR, then set CR0.PE.
;;
enter_protected_mode:
    cli
    lgdt    [gdt.ptr]
    lidt    [idt.ptr]
    mov     eax, cr0
    or      al, 1
    mov     cr0, eax
    jmp     BOOT_CODE_SEL:KERN_OFFSET

;;
;; This procedure prints string to the screen using BIOS INT 10h AH=0Eh
;; (teletype function) then halts when finished. It is assumed that SI points to
;; a string before it is called.
;;
error:
    mov   si, str_error
    mov   ax, 0x0003 ; AL=3 (80x25 16 color text video mode)
    int   0x10       ; Set the video mode using BIOS INT 10h AH=00h
    cld
    mov   bx, 0x000F ; Page 0 (DH), white foreground (DL)
    mov   ah, 0x0E
  .next_char:
    lodsb            ; Load byte from SI into AL
    test  al, al
    jz    .done      ; If AL is zero, we reached end of the string
    int   0x10
    jmp   .next_char
  .done:
    hlt

;;
;; Global Descriptor Table (GDT). Note that the first entry must be null.
;;
gdt:
  .null:
    dq    0
  .kern_code:
    dw    0xFFFF
    dw    0
    db    0
    db    0x9A ; P=1, DPL=00, S=1, Type=1010 (code r/x)
    db    0xAF ; G=1, D=0, L=1, AVL=0
    db    0
  .kern_data:
    dw    0xFFFF
    dw    0
    db    0
    db    0x92 ; P=1, DPL=00, S=1, Type=0010 (data r/w)
    db    0xCF ; G=1, D=1, L=0, AVL=0
    db    0
  .user_data:
    dw    0xFFFF
    dw    0
    db    0
    db    0xF2 ; P=1, DPL=11, S=1, Type=0010 (data r/w)
    db    0xCF ; G=1, D=1, L=0, AVL=0
    db    0
  .user_code:
    dw    0xFFFF
    dw    0
    db    0
    db    0xFA ; P=1, DPL=11, S=1, Type=1010 (code r/x)
    db    0xAF ; G=1, D=0, L=1, AVL=0
    db    0
  .boot_code:
    dw    0xFFFF
    dw    0
    db    0
    db    0x9A ; P=1, DPL=00, S=1, Type=1010 (code r/x)
    db    0xCF ; G=1, D=1, L=0, AVL=0
    db    0
  .ptr:
    dw    $ - gdt - 1 ; Limit
    dd    gdt         ; Base

;;
;; Interrupt Descriptor Table (IDT). WARNING - an empty IDT will cause all
;; NMIs to triple fault!
;;
idt:
  .ptr:
    dw    $ - gdt - 1 ; Limit
    dd    gdt         ; Base

;;
;; Disk Address Packets (DAP) - must be aligned on 4 byte boundary.
;;
dap:
    align 4
  .kern_code:
    db    0x10, 0x00
    dw    KERN_SECTORS ; FIXME: Not confirmed to fit in 2 bytes...
    dw    KERN_OFFSET
    dw    0
    dq    1

str_error:
    db    0x0D, 0x0A, "Error in MBR", 0

;;
;; MBR magic number so BIOS marks us bootable.
;;
times 510-($-$$) db 0
dw 0xAA55
