/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x40000
    RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x4000
}

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

/*
 * reset_handler must be global symbol as it is used in this script as entry point
 *
 * defined following symbols:
 *	 __driver_init_start__
 *	 __driver_init_end__
 * 	 __cmder_cmds_start__
 *   __cmder_cmds_end__
 *   __static_task_start__
 *	 __static_task_end__
 *   __exidx_start
 *   __exidx_end
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 */

ENTRY(reset_handler)

SECTIONS
    {
    .text :
        {
        KEEP(*(.vectors))

        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        *(.rodata*)

        . = ALIGN(4);
        __driver_init_start__ = .;
        KEEP(*(driver_init))
        __driver_init_end__ = .;

        . = ALIGN(4);
        __cmder_cmds_start__ = .;
        KEEP(*(cmder_cmds))
        __cmder_cmds_end__ = .;

        *(.eh_frame*)
        . = ALIGN(4);
        } > FLASH


    .ARM.extab :
        {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        . = ALIGN(4);
        } > FLASH

    __exidx_start = .;
    .ARM.exidx :
        {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        . = ALIGN(4);
        } > FLASH
    __exidx_end = .;

    __etext = .;

    .data : AT (__etext)
        {
        __data_start__ = .;
        *(vtable)
        *(.data*)

        . = ALIGN(4);
        __static_task_start__ = .;
        KEEP(*(static_task))
        __static_task_end__ = .;

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        *(SORT(.init_array.*))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        *(SORT(.fini_array.*))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        *(.jcr)
        . = ALIGN(4);
        /* All data end */
        __data_end__ = .;
    } > RAM

    .bss :
        {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
        } > RAM

    end     = .;
    __end__ = end;
    }

