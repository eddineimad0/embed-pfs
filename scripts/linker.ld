/* Define memory regions. */
MEMORY
{
	FLASH 	 (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
	SRAM 	 (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}


/* Libopencm3 will need these */
EXTERN (vector_table)
ENTRY(reset_handler)

/* Define sections. */
SECTIONS
{
    .text : {
        *(.vectors)      /* Vector_table */
        *(.text*)	/* Program code */
        . = ALIGN(4);
    } >FLASH

    .rodata :{
        *(.rodata*)	/* Read-only data */
    } >FLASH
    . = ALIGN(4);
    _etext = .;

    /* C++ Static constructors/destructors, also used for __attribute__
	 * ((constructor)) and the likes */
	.preinit_array : {
		. = ALIGN(4);
		__preinit_array_start = .;
		KEEP (*(.preinit_array))
		__preinit_array_end = .;
	} >FLASH
	.init_array : {
		. = ALIGN(4);
		__init_array_start = .;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__init_array_end = .;
	} >FLASH
	.fini_array : {
		. = ALIGN(4);
		__fini_array_start = .;
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
		__fini_array_end = .;
	} >FLASH

    .data :
    {
        _data = .;
        *(.data*) /* Read-write initialized data */
        . = ALIGN(4);
        _edata = .;
    } >SRAM AT> FLASH
    _data_loadaddr = LOADADDR(.data);

    .bss :
    {
        _bss = .;
        *(.bss*) /* Read-write zero initialized data */
        . = ALIGN(4);
        _ebss = .;
    } >SRAM

    . = ALIGN(4);
    end = .;
}

PROVIDE(_stack = ORIGIN(SRAM) + LENGTH(SRAM));
