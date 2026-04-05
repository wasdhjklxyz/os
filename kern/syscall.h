/**
 * Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef __SYSCALL_H
#define __SYSCALL_H

/**
 * When SYSCALL executes:
 *  RCX = RIP                 # Save return address
 *  R11 = RFLAGS              # Save flags
 *  RFLAGS &= ~SFMASK         # Mask flags
 *  CS.sel = STAR[47:32]      # Load kernel CS
 *  SS.sel = STAR[47:32] + 8  # Load kernel SS
 *  RIP = LSTAR               # Jump to kernel entry
 *  CPL = 0                   # Now in kernel mode
 *
 * When SYSRET executes:
 *  RIP = RCX                        # Restore user RIP
 *  RFLAGS[31:0] = R11[31:0]         # Restore flags (lower 32 bits)
 *  RFLAGS[63:32] = 0                # Clear upper bits
 *  CS.sel = (STAR[63:48] + 16) | 3  # Load user CS (0x10 + 16 = 0x20)
 *  SS.sel = (STAR[63:48] + 8) | 3   # Load user SS (0x10 + 8 = 0x18)
 *  CPL = 3                          # Now in user mode
 */
void syscall_init(void);

#endif // __SYSCALL_H
