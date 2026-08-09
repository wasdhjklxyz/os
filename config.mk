#
# Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
#
# SPDX-License-Identifier: BSD-2-Clause
#

BOOT  := boot
KERN  := kern
USER  := user
BUILD := build

# FIXME: Magic fucking numbers
KERN_OFFSET  := 0x8000
USER_OFFSET  := 0x600000
USER_LBA     := 32
USER_SECTORS := 1
DISK_SECTORS := 2048
