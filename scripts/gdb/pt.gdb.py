#
# Copyright (c) 2026, uiop <uiop@wasdhjkl.xyz>
#
# SPDX-License-Identifier: BSD-2-Clause
#

import gdb

LEVELS = ((4, 39, "$PML4"), (3, 30, "$PDP"), (2, 21, "$PD"), (1, 12, "$PT"))
NAME = {lvl: n for lvl, _, n in LEVELS}
SHIFT = {lvl: s for lvl, s, _ in LEVELS}

ADDR_MASK = 0x000FFFFFFFFFF000
P, RW, US, PS, NX = 1 << 0, 1 << 1, 1 << 2, 1 << 7, 1 << 63


def canon(va): # NOTE: Sign-extend bit 47
    va &= (1 << 48) - 1
    return va | 0xFFFF000000000000 if va & (1 << 47) else va


def physbase():
    v = gdb.parse_and_eval("$PHYSMAP_BASE")
    if v.type.code == gdb.TYPE_CODE_VOID:
        raise gdb.GdbError("$PHYSMAP_BASE unset (source build/config.gdb)")
    return int(v)


def rd(pa):
    return int(gdb.parse_and_eval(f"*(unsigned long *){pa + physbase()}"))


def evalint(expr):
    v = gdb.parse_and_eval(expr)
    try:
        return int(v)
    except gdb.error:
        return int(v.address)


def flags(e):
    s = "P" if e & P else "-"
    s += "W" if e & RW else "R"
    s += "U" if e & US else "S"
    s += "-" if e & NX else "X"
    return s + (" 1G" if e & PS else "")


def target(e, va, shift):
    off = (1 << shift) - 1
    return (e & ADDR_MASK & ~off) | (va & off)


class PtWalk(gdb.Command):
    """ptwalk <va> [root]  -- walk one VA down the tables.

    root defaults to $cr3; pass &pml4 to inspect a table not yet loaded."""

    def __init__(self):
        super().__init__("ptwalk", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        argv = gdb.string_to_argv(arg)
        if not argv:
            print(PtWalk.__doc__)
            return

        va = canon(evalint(argv[0]))
        tbl = evalint(argv[1] if len(argv) > 1 else "$cr3") & ~0xFFF
        print(f"va {va:#018x}")

        for lvl, shift, name in LEVELS:
            idx = (va >> shift) & 0x1FF
            ent = rd(tbl + idx * 8)
            print(f"  {name:<4}[{idx:3}] = {ent:#018x}  {flags(ent)}")

            if not ent & P:
                print("  not present")
                return
            if lvl > 1 and ent & PS:
                print(f"  -> pa {target(ent, va, shift):#x}  ({1 << (shift-20)}M page)")
                return
            tbl = ent & ADDR_MASK

        print(f"  -> pa {target(ent, va, 12):#x}")


class PtDump(gdb.Command):
    """ptdump [root] [maxdepth]  -- dump the whole table tree.

    root defaults to $cr3. maxdepth limits recursion (default 4)."""

    def __init__(self):
        super().__init__("ptdump", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        argv = gdb.string_to_argv(arg)
        tbl = evalint(argv[0] if argv else "$cr3") & ~0xFFF
        depth = int(argv[1]) if len(argv) > 1 else 4
        print(f"root @ {tbl:#x}")
        self.dump(tbl, 4, 0, depth)

    def dump(self, tbl, lvl, prefix, depth):
        shift, name = SHIFT[lvl], NAME[lvl]
        indent = "  " * (5 - lvl)
        seen = 0

        for i in range(512):
            ent = rd(tbl + i * 8)
            if not ent & P:
                continue
            seen += 1
            va = canon(prefix | (i << shift))
            leaf = lvl == 1 or ent & PS
            arrow = f"pa {ent & ADDR_MASK:#x}"
            print(f"{indent}{name:<4}[{i:3}] va {va:#018x} -> {arrow}  {flags(ent)}")
            if not leaf and 5 - lvl < depth:
                self.dump(ent & ADDR_MASK, lvl - 1, va, depth)

        if seen == 0:
            print(f"{indent}(empty)")


PtWalk()
PtDump()
