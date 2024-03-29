/* Same as flash.ld but everything is linked in internal flash */

MEMORY {
  INTERNAL_FLASH_ITCM (rx) : ORIGIN = 0x00200000, LENGTH = 64K
  INTERNAL_FLASH_AXIM (rx) : ORIGIN = 0x08000000, LENGTH = 64K
  SRAM (rw)  : ORIGIN = 0x20000000, LENGTH = 256K
}

STACK_SIZE = 32K;
TRAMPOLINES_OFFSET = 0xE000;
CUSTOM_TRAMPOLINES_OFFSET = 64K - 64;
FLASH_SECOND_SECTOR_OFFSET = 16K;
FLASH_SECOND_SECTOR_SIZE = 16K;

SECTIONS {
  .isr_vector_table ORIGIN(INTERNAL_FLASH_ITCM) : {
    /* When booting, the STM32F412 fetches the content of address 0x0, and
     * extracts from it various key infos: the initial value of the PC register
     * (program counter), the initial value of the stack pointer, and various
     * entry points to interrupt service routines. This data is called the ISR
     * vector table.
     *
     * Note that address 0x0 is always an alias. It points to the beginning of
     * Flash, SRAM, or integrated bootloader depending on the boot mode chosen.
     * (This mode is chosen by setting the BOOTn pins on the chip).
     *
     * We're generating the ISR vector table in code because it's very
     * convenient: using function pointers, we can easily point to the service
     * routine for each interrupt. */
    KEEP(*(.isr_vector_table))
  } >INTERNAL_FLASH_ITCM

  .header : {
    KEEP(*(.header))
  } >INTERNAL_FLASH_ITCM

  .rodata : {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata.*)
  } >INTERNAL_FLASH_ITCM

  .text.before : {
    . = ALIGN(4);
    /*
     * We move some stuff here because there are 6k of free space between the
     * end of .rodata and the beginning of .exam_mode_buffer
     */
    *(.text.ed25519_*)
    *(.text.sha512_*)
    *(.text.fe_*)
    *(.text.sc_*)
    *(.text.ge_*)
    *(.text.consttime_equal)
  } >INTERNAL_FLASH_ITCM

  .exam_mode_buffer ORIGIN(INTERNAL_FLASH_ITCM) + FLASH_SECOND_SECTOR_OFFSET : {
    _exam_mode_buffer_start = ORIGIN(INTERNAL_FLASH_AXIM) + FLASH_SECOND_SECTOR_OFFSET;
    KEEP(*(.exam_mode_buffer))
    /* Note: We don't increment "." here, we set it. */
    . = ORIGIN(INTERNAL_FLASH_ITCM) + FLASH_SECOND_SECTOR_OFFSET + FLASH_SECOND_SECTOR_SIZE;
    _exam_mode_buffer_end = ORIGIN(INTERNAL_FLASH_AXIM) + FLASH_SECOND_SECTOR_OFFSET + FLASH_SECOND_SECTOR_SIZE;
  } >INTERNAL_FLASH_ITCM

  .text : {
    . = ALIGN(4);
    *(.text)
    *(.text.*)
  } >INTERNAL_FLASH_ITCM

  .init_array : {
    . = ALIGN(4);
    _init_array_start = .;
    KEEP (*(.init_array*))
    _init_array_end = .;
    ASSERT(_init_array_start == _init_array_end, "Error: C++ Static constructors are not allowed");
  } >INTERNAL_FLASH_ITCM

  .data : {
    /* The data section is written to Flash but linked as if it were in RAM.
     *
     * This is required because its initial value matters (so it has to be in
     * persistant memory in the first place), but it is a R/W area of memory
     * so it will have to live in RAM upon execution (in linker lingo, that
     * translates to the data section having a LMA in Flash and a VMA in RAM).
     *
     * This means we'll have to copy it from Flash to RAM on initialization.
     * To do this, we'll need to know the source location of the data section
     * (in Flash), the target location (in RAM), and the size of the section.
     * That's why we're defining three symbols that we'll use in the initial-
     * -ization routine. */
    . = ALIGN(4);
    _data_section_start_flash = LOADADDR(.data);
    _data_section_start_ram = .;
    *(.data)
    *(.data.*)
    _data_section_end_ram = .;
  } >SRAM AT> INTERNAL_FLASH_ITCM

  .trampolines_table : {
    . = ORIGIN(INTERNAL_FLASH_ITCM) + TRAMPOLINES_OFFSET;
    KEEP(*(.trampolines_table));
  } > INTERNAL_FLASH_ITCM

  .custom_trampolines_table : {
    . = ORIGIN(INTERNAL_FLASH_ITCM) + CUSTOM_TRAMPOLINES_OFFSET;
    KEEP(*(.custom_trampolines_table));
  } > INTERNAL_FLASH_ITCM

  .bss : {
    /* The bss section contains data for all uninitialized variables
     * So like the .data section, it will go in RAM, but unlike the data section
     * we don't care at all about an initial value.
     *
     * Before execution, crt0 will erase that section of memory though, so we'll
     * need pointers to the beginning and end of this section. */
    . = ALIGN(4);
    _bss_section_start_ram = .;
    *(.bss)
    *(.bss.*)
    /* The compiler may choose to allocate uninitialized global variables as
     * COMMON blocks. This can be disabled with -fno-common if needed. */
    *(COMMON)
    _bss_section_end_ram = .;
  } >SRAM

  .heap : {
    _heap_start = .;
    /* Note: We don't increment "." here, we set it. */
    . = (ORIGIN(SRAM) + LENGTH(SRAM) - STACK_SIZE);
    _heap_end = .;
  } >SRAM

  .stack : {
    . = ALIGN(8);
    _stack_end = .;
    . += (STACK_SIZE - 8);
    . = ALIGN(8);
    _stack_start = .;
  } >SRAM
}