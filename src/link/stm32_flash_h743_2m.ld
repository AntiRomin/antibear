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
0x24000000 to 0x2407FFFF  512K AXI SRAM, D1 domain, main RAM
0x30000000 to 0x3001FFFF  128K SRAM1, D2 domain
0x30020000 to 0x3003FFFF  128K SRAM2, D2 domain
0x30040000 to 0x30047FFF   32K SRAM3, D2 domain
0x38000000 to 0x3800FFFF   64K SRAM4, D3 domain
0x38800000 to 0x38800FFF    4K BACKUP SRAM, Backup domain

0xD0000000 to 0xD3FFFFFF   64M SDRAM_B2, FMC SDRAM BANK2

0x08000000 to 0x081FFFFF 2048K full flash,
0x08000000 to 0x0801FFFF  128K isr vector, startup code,
0x08020000 to 0x0803FFFF  128K config,                  // FLASH_Sector_1
0x08040000 to 0x081FFFFF 1792K firmware,
*/

/* Specify the memory areas */
MEMORY
{
    FLASH (rx)        : ORIGIN = 0x08000000, LENGTH = 128K
    FLASH_CONFIG (r)  : ORIGIN = 0x08020000, LENGTH = 128K
    FLASH1 (rx)       : ORIGIN = 0x08040000, LENGTH = 1792K

    ITCM_RAM (xrw)    : ORIGIN = 0x00000000, LENGTH = 64K
    DTCM_RAM (xrw)    : ORIGIN = 0x20000000, LENGTH = 128K
    RAM (xrw)         : ORIGIN = 0x24000000, LENGTH = 512K

    RAM_D2 (xrw)      : ORIGIN = 0x30000000, LENGTH = 288K

    RAM_D3 (xrw)      : ORIGIN = 0x38000000, LENGTH = 64K

    RAM_BACKUP (xrw)  : ORIGIN = 0x38800000, LENGTH = 4K

    SDRAM_B2 (xrw)    : ORIGIN = 0xD0000000, LENGTH = 64M
}

REGION_ALIAS("STACKRAM", DTCM_RAM)
REGION_ALIAS("FASTRAM", DTCM_RAM)

/* Highest address of the user mode stack */
_estack = ORIGIN(STACKRAM) + LENGTH(STACKRAM); /* end of RAM */

/* Base address where the config is stored. */
__config_start = ORIGIN(FLASH_CONFIG);
__config_end = ORIGIN(FLASH_CONFIG) + LENGTH(FLASH_CONFIG);

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x2000;  /* required amount of heap  */
_Min_Stack_Size = 0x2000; /* required amount of stack */

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
  } >FLASH1

  /* Critical program code goes into ITCM RAM */
  /* Copy specific fast-executing code to ITCM RAM */
  tcm_code = LOADADDR(.tcm_code);
  .tcm_code :
  {
    . = ALIGN(4);
    tcm_code_start = .;
    *(.tcm_code)
    *(.tcm_code*)
    . = ALIGN(4);
    tcm_code_end = .;
  } >ITCM_RAM AT >FLASH1

  .ARM.extab   :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH1

  .ARM :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH1

  .pg_registry :
  {
    PROVIDE_HIDDEN (__pg_registry_start = .);
    KEEP (*(.pg_registry))
    KEEP (*(SORT(.pg_registry.*)))
    PROVIDE_HIDDEN (__pg_registry_end = .);
  } >FLASH1

  .pg_resetdata :
  {
    PROVIDE_HIDDEN (__pg_resetdata_start = .);
    KEEP (*(.pg_resetdata))
    PROVIDE_HIDDEN (__pg_resetdata_end = .);
  } >FLASH1

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
  } >RAM AT >FLASH1

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

  /* used during startup to initialized fastram_data */
  _sfastram_idata = LOADADDR(.fastram_data);

  /* Initialized FAST_DATA section for unsuspecting developers */
  .fastram_data :
  {
    . = ALIGN(4);
    _sfastram_data = .;        /* create a global symbol at data start */
    *(.fastram_data)           /* .data sections */
    *(.fastram_data*)          /* .data* sections */

    . = ALIGN(4);
    _efastram_data = .;        /* define a global symbol at data end */
  } >FASTRAM AT >FLASH1

  . = ALIGN(4);
  .fastram_bss (NOLOAD) :
  {
    _sfastram_bss = .;
    __fastram_bss_start__ = _sfastram_bss;
    *(.fastram_bss)
    *(SORT_BY_ALIGNMENT(.fastram_bss*))

    . = ALIGN(4);
    _efastram_bss = .;
    __fastram_bss_end__ = _efastram_bss;
  } >FASTRAM

  /* used during startup to initialized dmaram_data */
  _sdmaram_idata = LOADADDR(.dmaram_data);

  . = ALIGN(32);
  .dmaram_data :
  {
    PROVIDE(dmaram_start = .);
    _sdmaram = .;
    _dmaram_start__ = _sdmaram;
    _sdmaram_data = .;        /* create a global symbol at data start */
    *(.dmaram_data)           /* .data sections */
    *(.dmaram_data*)          /* .data* sections */
    . = ALIGN(32);
    _edmaram_data = .;        /* define a global symbol at data end */
  } >RAM AT >FLASH1

  . = ALIGN(32);
  .dmaram_bss (NOLOAD) :
  {
    _sdmaram_bss = .;
    __dmaram_bss_start__ = _sdmaram_bss;
    *(.dmaram_bss)
    *(SORT_BY_ALIGNMENT(.dmaram_bss*))
    . = ALIGN(32);
    _edmaram_bss = .;
    __dmaram_bss_end__ = _edmaram_bss;
  } >RAM

  . = ALIGN(32);
  .DMA_RAM (NOLOAD) :
  {
    KEEP(*(.DMA_RAM))
    PROVIDE(dmaram_end = .);
    _edmaram = .;
    _dmaram_end__ = _edmaram;
  } >RAM

  .DMA_RW_AXI (NOLOAD) :
  {
    . = ALIGN(32);
    PROVIDE(dmarwaxi_start = .);
    _sdmarwaxi = .;
    _dmarwaxi_start__ = _sdmarwaxi;
    KEEP(*(.DMA_RW_AXI))
    PROVIDE(dmarwaxi_end = .);
    _edmarwaxi = .;
    _dmarwaxi_end__ = _edmarwaxi;
  } >RAM

  .persistent_data (NOLOAD) :
  {
    __persistent_data_start__ = .;
    *(.persistent_data)
    . = ALIGN(4);
    __persistent_data_end__ = .;
  } >RAM

  /* used during startup to initialized sdram2_data */
  _ssdram2_idata = LOADADDR(.sdram2_data);

  /* Initialized FAST_DATA section for unsuspecting developers */
  .sdram2_data :
  {
    . = ALIGN(4);
    _ssdram2_data = .;        /* create a global symbol at data start */
    *(.sdram2_data)           /* .data sections */
    *(.sdram2_data*)          /* .data* sections */

    . = ALIGN(4);
    _esdram2_data = .;        /* define a global symbol at data end */
  } >SDRAM_B2 AT >FLASH

  . = ALIGN(4);
  .sdram2_bss (NOLOAD) :
  {
    _ssdram2_bss = .;
    __sdram2_bss_start__ = _ssdram2_bss;
    *(.sdram2_bss)
    *(SORT_BY_ALIGNMENT(.sdram2_bss*))

    . = ALIGN(4);
    _esdram2_bss = .;
    __sdram2_bss_end__ = _esdram2_bss;
  } >SDRAM_B2

  .DMA_VRAM (NOLOAD) :
  {
    . = ALIGN(32);
    PROVIDE(dmavran_start = .);
    _sdmavram = .;
    _dmavram_start__ = _sdmavram;
    KEEP(*(.DMA_VRAM))
    PROVIDE(dmavram_end = .);
    _edmavram = .;
    _dmavram_end__ = _edmavram;
  } >SDRAM_B2

  /* User_heap_stack section, used to check that there is enough RAM left */
  _heap_stack_end = ORIGIN(STACKRAM) + LENGTH(STACKRAM);
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
  } >STACKRAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
