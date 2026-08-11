ENTRY(_start)
SECTIONS {
  . = KERN_OFFSET;

  /* Runs in 32-bit pmode before paging reaches the higher half, so VMA == LMA
   * here. This is the only code linked at a physical address. */
  .text.boot ALIGN(0x1000) : {
    *(.text.boot)
    . = ALIGN(0x1000);
  }

  __kern_lma = .;
  . = KERN_VMA + __kern_lma;

  .text ALIGN(0x1000) : AT(ADDR(.text) - KERN_VMA) {
    __text_start = .;
    *(.text*)
    . = ALIGN(0x1000);
    __text_end = .;
  }

  .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - KERN_VMA) {
    __rodata_start = .;
    *(.rodata*)
    . = ALIGN(0x1000);
    __rodata_end = .;
  }

  .data ALIGN(0x1000) : AT(ADDR(.data) - KERN_VMA) {
    __data_start = .;
    *(.data*)
    . = ALIGN(0x1000);
    __data_end = .;
  }

  .bss ALIGN(0x1000) : AT(ADDR(.bss) - KERN_VMA) {
    __bss_start = .;
    *(.bss*) *(COMMON)
    . = ALIGN(0x1000);
    __bss_end = .;
  }

  /DISCARD/ : { *(.eh_frame) *(.note*) *(.comment) }

  __kern_end = .;
  __kern_end_phys = . - KERN_VMA;
}
