/*
*****************************************************************************
**
**  File        : stm32_flash_h7x3_2m.ld
**
**  Abstract    : Linker script for STM32H743xI Device with
**                512K AXI-RAM mapped onto AXI bus on D1 domain
**                128K SRAM1 mapped on D2 domain
**                128K SRAM2 mapped on D2 domain
**                 32K SRAM3 mapped on D2 domain
**                 64K SRAM4 mapped on D3 domain
**                 64K ITCM
**                128K DTCM
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/*
0x00000000 to 0x0000FFFF   64K ITCM
0x20000000 to 0x2001FFFF  128K DTCM
0x24000000 to 0x2405FFFF  384K AXI SRAM, D1 domain, main RAM
0x30000000 to 0x30007FFF   32K SRAM1, D2 domain
0x30020000 to 0x30003FFF   16K SRAM2, D2 domain
0x38000000 to 0x3800FFFF   64K SRAM4, D3 domain
0x38800000 to 0x38800FFF    4K BACKUP SRAM, Backup domain

0x08000000 to 0x081FFFFF 2048K full flash,
0x08000000 to 0x081FFFFF 2048K isr vector, startup code, config, firmware
*/

/* Specify the memory areas */
MEMORY
{
    FLASH (rx)        : ORIGIN = 0x08000000, LENGTH = 2048K

    ITCM_RAM (rwx)    : ORIGIN = 0x00000000, LENGTH = 64K
    DTCM_RAM (rwx)    : ORIGIN = 0x20000000, LENGTH = 128K
    RAM (rwx)         : ORIGIN = 0x24000000, LENGTH = 384K

    D2_RAM1 (rwx)     : ORIGIN = 0x30000000, LENGTH = 32K
    D2_RAM2 (rwx)     : ORIGIN = 0x30020000, LENGTH = 16K

    D3_RAM (rwx)      : ORIGIN = 0x38000000, LENGTH = 64K

    BACKUP_RAM (rwx)  : ORIGIN = 0x38800000, LENGTH = 4K
}

REGION_ALIAS("STACKRAM", DTCM_RAM)
REGION_ALIAS("FASTRAM", DTCM_RAM)

/* Highest address of the user mode stack */
_estack = ORIGIN(STACKRAM) + LENGTH(STACKRAM) - 8; /* Reserve 2 x 4bytes for info across reset */

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0;      /* required amount of heap  */
_Min_Stack_Size = 0x800; /* required amount of stack */

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(512);
    PROVIDE (isr_vector_table_base = .);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  .ARM.extab   :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH

  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*) __exidx_end = .;
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM AT >FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss (NOLOAD) :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(SORT_BY_ALIGNMENT(.bss*))
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  _heap_stack_end = ORIGIN(STACKRAM) + LENGTH(STACKRAM) - 8; /* 8 bytes to allow for alignment */
  _heap_stack_begin = _heap_stack_end - _Min_Stack_Size  - _Min_Heap_Size;
  . = _heap_stack_begin;
  ._user_heap_stack :
  {
    . = ALIGN(4);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(4);
  } >STACKRAM = 0xa5

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}