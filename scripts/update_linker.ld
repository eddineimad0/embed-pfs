/* Define memory regions. */
MEMORY
{
        /* The linker should offset address by 0x08004048,
         * 0x4000 for the bootloader on the microcontroller
         * and 0x48 for the firmware infos that will be added
         * to the begining of the update binary.
         * */
	FLASH 	 (rx)  : ORIGIN = 0x08004000, LENGTH = 48K 
	SRAM 	 (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}


/* Libopencm3 will need these */
/* Enforce emmition of the vector table. */
EXTERN (vector_table)

/* Define the entry point of the output file. */
ENTRY(reset_handler)

/* Define sections. */
SECTIONS
{
    .text : {
            *(.vectors)	/* Vector table */
            *(.text*)	/* Program code */
            . = ALIGN(4);
            *(.rodata*)	/* Read-only data */
            . = ALIGN(4);
    } >FLASH

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

    /*
        * Another section used by C++ stuff, appears when using newlib with
        * 64bit (long long) printf support
        */
    .ARM.extab : {
            *(.ARM.extab*)
    } >FLASH
    .ARM.exidx : {
            __exidx_start = .;
            *(.ARM.exidx*)
            __exidx_end = .;
    } >FLASH

    . = ALIGN(4);
    _etext = .;

    /* ram, but not cleared on reset, eg boot/app comms */
    .noinit (NOLOAD) : {
            *(.noinit*)
    } >SRAM
    . = ALIGN(4);

    .data : {
            _data = .;
            *(.data*)	/* Read-write initialized data */
            *(.ramtext*)    /* "text" functions to run in ram */
            . = ALIGN(4);
            _edata = .;
    } >SRAM AT >FLASH
    _data_loadaddr = LOADADDR(.data);

    .bss : {
            *(.bss*)	/* Read-write zero initialized data */
            *(COMMON)
            . = ALIGN(4);
            _ebss = .;
    } >SRAM

    /*
        * The .eh_frame section appears to be used for C++ exception handling.
        * You may need to fix this if you're using C++.
        */
    /DISCARD/ : { *(.eh_frame) }

    . = ALIGN(4);
    end = .;
}

PROVIDE(_stack = ORIGIN(SRAM) + LENGTH(SRAM));
