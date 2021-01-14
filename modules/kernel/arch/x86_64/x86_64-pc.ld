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

KERN_CODE = 0xffffffff80000000;

SECTIONS {
    . = KERN_CODE;                           /* Multiboot will place the kernel here in physical memory */


    /*
     * -- This is where the kernel code will be placed.
     *    ---------------------------------------------
     */
    .text : {
        *(.init)
        *(.text .text.* .stub .gnu.linkonce.t.*)
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        . = ALIGN(4096);

    }


    /*
     * -- We drop in the read/write data here
     *    -----------------------------------
     */
    .data : {
        *(.data)
        . = ALIGN(8);

        *(.bss)
        . = ALIGN(4096);
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
