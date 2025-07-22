/*
 * ARM64 OS Linker Script
 * Memory layout starting at 0x40000000 (QEMU virt default)
 */

ENTRY(_start)

MEMORY
{
    /* QEMU virt machine RAM starts at 0x40000000 */
    RAM : ORIGIN = 0x40000000, LENGTH = 128M
}

SECTIONS
{
    . = 0x40000000;
    
    /* Boot code section - must be first */
    .text.boot : {
        KEEP(*(.text.boot))
    } > RAM
    
    /* Main code section */
    .text : {
        *(.text)
        *(.text.*)
    } > RAM
    
    /* Read-only data */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } > RAM
    
    /* Initialized data */
    .data : {
        *(.data)
        *(.data.*)
    } > RAM
    
    /* Uninitialized data (BSS) */
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    } > RAM
    
    /* End of image marker */
    . = ALIGN(4096);
    _end = .;
    
    /* Discard debug sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.debug*)
    }
}