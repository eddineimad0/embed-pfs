/* Define memory regions. */
MEMORY
{
	FLASH 	 (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
	SRAM 	 (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* Enforce emmition of the vector table. */
EXTERN (vector_table)

/* Define the entry point of the output file. */
ENTRY(reset_handler)

/* Define sections. */
SECTIONS
{
    .text : {
        *(.text*)	/* Program code */
        _etext = .;
    } >FLASH

    .rodata :{
        *(.rodata*)	/* Read-only data */
    } >FLASH

    _data_loadaddr = LOADADDR(.data);
    .data :
    {
        _sdata = .;
        *(.data*) /* Read-write initialized data */
        _edata = .;
    } >SRAM AT> FLASH

    .bss :
    {
        _sbss = .;
        *(.bss*) /* Read-write zero initialized data */
        _ebss = .;
    } >SRAM
}

PROVIDE(_stack = ORIGIN(SRAM) + LENGTH(SRAM));
