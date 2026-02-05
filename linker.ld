ENTRY(_start)

SECTIONS
{
    /* Multiboot2 header must be in first 32 KiB */
    .multiboot_header ALIGN(8) :
    {
        KEEP(*(.multiboot_header))
    }

    /* Identity-mapped kernel load address */
    . = 1M;

    .text ALIGN(16) :
    {
        *(.text*)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata*)
    }

    .data ALIGN(4K) :
    {
        *(.data*)
    }

    .bss ALIGN(16) :
    {
        *(COMMON)
        *(.bss*)
    }
}

