OUTPUT_FORMAT(elf_x86_64)
ENTRY(start)
SECTIONS
 {
   . = 1M;
   .text BLOCK(4K) : ALIGN(4K)
   {
       *(.multiboot)
       *(.text)
   }
   .data : { *(.data) }
   .bss  : { *(.bss)  }
 }

