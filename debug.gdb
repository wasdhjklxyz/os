set confirm off
set disassembly-flavor intel

target remote :1234

add-symbol-file kern.elf
add-symbol-file user.elf

# break *0x7C00
# break kern_start
# break user.c:main

# continue
