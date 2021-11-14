/*******************************************************************************************************************/
/*                                                                                                                 */
/*  kernel.ld -- This is the linker script for locating the sections in the target binary                          */
/*                                                                                                                 */
/*        Copyright (c)  2017-2021 -- Adam Clark                                                                   */
/*                                                                                                                 */
/* --------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */
/*     Date      Tracker  Version  Pgmr  Description                                                               */
/*  -----------  -------  -------  ----  ------------------------------------------------------------------------  */
/*  2021-Jan-03  Initial  v0.0.1   ADCL  Initial version                                                           */
/*                                                                                                                 */
/*******************************************************************************************************************/

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(entry)

PHYS = 0x00100000;

SECTIONS {
    . = PHYS;                           /* Multiboot will place the kernel here in physical memory */

    /*
     * -- Start with the multiboot required code, and all the entry code needed to get the MMU initialized
     *    ------------------------------------------------------------------------------------------------
     */
    .entry : {
        *(.mboot)
        *(.text .text.*)
        *(.rodata .rodata.*)
        *(.data .data.*)
        *(.bss .bss.*)
        . = ALIGN(4096);
        __loaderEnd = .;
    }

    /*
     * -- Include debugging information in loader memory
     *    ----------------------------------------------
     */
    .stab : {
        *(.stab);
        *(.stabstr);
        *(.debug*);
        *(.got*)
        BYTE(0)        /* Force the linker to allocate space for this section */
        . = ALIGN(4096);
    }


    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack .comment .bootstack)
    }
}
