/****************************************************************************************************************//**
*   @file               mmu-kernel.cc
*   @brief              Kernel-specific functions related to managing the Paging Tables
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Nov-07
*   @since              v0.0.13
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Nov-07 | Initial |  v0.0.13 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "kernel-funcs.h"
#include "printf.h"
#include "mmu.h"



/********************************************************************************************************************
*   Documented in `mmu-funcs.h`
*///-----------------------------------------------------------------------------------------------------------------
Return_t krn_MmuMapPageEx(Addr_t space, Addr_t a, Frame_t f, int flags)
{
#if DEBUG_ENABLED(krn_MmuMapPageEx)

    kprintf("Preparing to map a page in another address space\n");

#endif

    Addr_t cr3 = GetAddressSpace();
    LoadCr3(space);
    int rv = cmn_MmuMapPage(a, f, flags);

    LoadCr3(cr3);

#if DEBUG_ENABLED(krn_MmuMapPageEx)

    kprintf("..done\n");

#endif

    return rv;
}



/********************************************************************************************************************
*   Documented in `mmu-funcs.h`
*///-----------------------------------------------------------------------------------------------------------------
Return_t krn_MmuUnmapEx(Addr_t space, Addr_t a)
{
    Addr_t cr3 = GetAddressSpace();
    LoadCr3(space);

    int rv = cmn_MmuUnmapPage(a);

    LoadCr3(cr3);

    return rv;
}



/********************************************************************************************************************
*   Documented in `mmu-funcs.h`
*///-----------------------------------------------------------------------------------------------------------------
Return_t krn_MmuDump(Addr_t addr)
{
    kprintf("\nMmuDump: Walking the page tables for address %p\n", addr);
    kprintf("Level  Entry Address       Index               Next Frame          us  rw  p\n");
    kprintf("-----  ------------------  ------------------  ------------------  --  --  -\n");

    uint64_t idx = (addr >> (12 + (9 * 3))) & 0x1ff;
    PageEntry_t *ent = GetPML4Entry(addr);

    kprintf("PML4   ");
    kprintf("%p  ", ent);
    kprintf("%p  ", idx);
    kprintf("%p  ", ent->frame);
    kprintf("%s   ", ent->us ? "1" : "0");
    kprintf("%s   ", ent->rw ? "1" : "0");
    kprintf("%s\n", ent->p ? "1" : "0");

    if (!ent->p) goto exit;


    idx = (addr >> (12 + (9 * 2))) & 0x1ff;
    ent = GetPDPTEntry(addr);

    kprintf("PDPT   ");
    kprintf("%p  ", ent);
    kprintf("%p  ", idx);
    kprintf("%p  ", ent->frame);
    kprintf("%s   ", ent->us ? "1" : "0");
    kprintf("%s   ", ent->rw ? "1" : "0");
    kprintf("%s\n", ent->p ? "1" : "0");

    if (!ent->p) goto exit;


    idx = (addr >> (12 + (9 * 1))) & 0x1ff;
    ent = GetPDEntry(addr);

    kprintf("  PD   ");
    kprintf("%p  ", ent);
    kprintf("%p  ", idx);
    kprintf("%p  ", ent->frame);
    kprintf("%s   ", ent->us ? "1" : "0");
    kprintf("%s   ", ent->rw ? "1" : "0");
    kprintf("%s\n", ent->p ? "1" : "0");

    if (!ent->p) goto exit;


    idx = (addr >> (12 + (9 * 0))) & 0x1ff;
    ent = GetPTEntry(addr);

    kprintf("  PT   ");
    kprintf("%p  ", ent);
    kprintf("%p  ", idx);
    kprintf("%p  ", ent->frame);
    kprintf("%s   ", ent->us ? "1" : "0");
    kprintf("%s   ", ent->rw ? "1" : "0");
    kprintf("%s\n", ent->p ? "1" : "0");

exit:
    kprintf("\n");
    return 0;
}


