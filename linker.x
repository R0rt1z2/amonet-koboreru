OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(start)

/* the 4K below the payload is unused SRAM, so the stack grows down into it. */
_stack_top = PAYLOAD_ADDR;

SECTIONS
{
  . = PAYLOAD_ADDR;

  .text     : { KEEP(*(.text.start)) *(.text   .text.*   .gnu.linkonce.t.*) }
  .rodata   : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
  .data     : { *(.data   .data.*   .gnu.linkonce.d.*) }
  . = ALIGN(4);
  .bss      : {
    __bss_start = .;
    *(.bss    .bss.*    .gnu.linkonce.b.*) *(COMMON)
    . = ALIGN(4);
    __bss_end = .;
  }
  /DISCARD/ : { *(.interp) *(.dynsym) *(.dynstr) *(.hash) *(.dynamic) *(.comment) }

  ASSERT(. <= BSS_START,
         "payload is too large,: it overlaps the forged BSS!!")
}
