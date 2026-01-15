ENTRY(_start)

SECTIONS
{
    .text : {
        *(.text.startup)
        *(.text*)
    }

    .data : {
        *(.data)
        *(.rodata*)
    }

    .bss : {
        *(.bss)
    }
}