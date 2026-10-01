# uiopOS

Hobby operating system for x86-64.

Still in early development. Use in a VM, booting on hardware is at your own
risk.

## Dependencies

If using Nix, a dev shell is provided. Run `nix develop`.

### Required

`gcc` `binutils` `nasm` `gnumake` `coreutils` `sed` `findutils`

### Optional

`qemu` `gdb`

## Configuration

Addresses, sizes, and qemu memory live in `config`.

> [!WARNING]
> Any changes you make to `config` are not guaranteed to work. Only the defaults
> have been tested.

## Build

Build using `make`. The disk image is at `build/disk.img`.

## Run

Boot using `make qemu`. It starts halted with a gdb stub on `:1234`. Serial
output goes to the terminal.

## Debug

In a second terminal, run `make debug`. This attaches gdb, loads symbols, and
sources `scripts/gdb/pt.gdb.py`, which adds two commands:

- `ptwalk <va> [root]`: walk one virtual address down the page tables
- `ptdump [root] [maxdepth]`: dump the whole page table tree

`root` defaults to `CR3`.

## License

BSD-2-Clause

