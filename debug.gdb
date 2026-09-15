set confirm off
set disassembly-flavor intel

target remote :1234

add-symbol-file build/kern/kern.elf
add-symbol-file build/user/user.elf

source build/config.gdb
