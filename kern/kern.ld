ENTRY(_start)
SECTIONS {
  . = KERN_OFFSET;

  .text ALIGN(0x1000): {
    __text_start = .;
    *(.text.boot) *(.text*)
    . = ALIGN(0x1000);
    __text_end = .;
  }

  .rodata ALIGN(0x1000): {
    __rodata_start = .;
    *(.rodata*)
    . = ALIGN(0x1000);
    __rodata_end = .;
  }

  .data ALIGN(0x1000): {
    __data_start = .;
    *(.data*)
    . = ALIGN(0x1000);
    __data_end = .;
  }

  .bss ALIGN(0x1000): {
    __bss_start = .;
    *(.bss*) *(COMMON)
    . = ALIGN(0x1000);
    __bss_end = .;
  }

  /DISCARD/ : { *(.eh_frame) *(.note*) *(.comment) }

  __kern_end = .;
}
