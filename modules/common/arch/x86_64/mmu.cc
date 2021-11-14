/****************************************************************************************************************//**
*   @file               mmu.cc
*   @brief              Functions related to managing the Paging Tables
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Jan-03
*   @since              v0.0.01
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Jan-03 | Initial |  v0.0.01 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "serial.h"
#include "mmu.h"



/********************************************************************************************************************
*   Documented in `mmu-arch.h`
*///-----------------------------------------------------------------------------------------------------------------
PageEntry_t *GetPML4Entry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)PML4_ENTRY_ADDRESS;
    uint64_t idx = (a >> (12 + (9 * 3))) & 0x1ff;

#if DEBUG_ENABLED(GetPML4Entry)

    SerialPutString("PML4 idx = ");
    SerialPutHex64(idx);
    SerialPutChar('\n');

#endif

    return &t[idx];
}



/********************************************************************************************************************
*   Documented in `mmu-arch.h`
*///-----------------------------------------------------------------------------------------------------------------
PageEntry_t *GetPDPTEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)PDPT_ENTRY_ADDRESS;
    uint64_t idx = (a >> (12 + (9 * 2))) & 0x3ffff;

#if DEBUG_ENABLED(GetPDPTEntry)

    SerialPutString("PDPT idx = ");
    SerialPutHex64(idx);
    SerialPutChar('\n');

#endif

    return &t[idx];
}



/********************************************************************************************************************
*   Documented in `mmu-arch.h`
*///-----------------------------------------------------------------------------------------------------------------
PageEntry_t *GetPDEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)PD_ENTRY_ADDRESS;
    uint64_t idx = (a >> (12 + (9 * 1))) & 0x7ffffff;

#if DEBUG_ENABLED(GetPDEntry)

    SerialPutString("PD idx = ");
    SerialPutHex64(idx);
    SerialPutChar('\n');

#endif

    return &t[idx];
}



/********************************************************************************************************************
*   Documented in `mmu-arch.h`
*///-----------------------------------------------------------------------------------------------------------------
PageEntry_t *GetPTEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)PT_ENTRY_ADDRESS;
    uint64_t idx = (a >> (12 + (9 * 0))) & 0xfffffffff;

#if DEBUG_ENABLED(GetPTEntry)

    SerialPutString("PT idx = ");
    SerialPutHex64(idx);
    SerialPutChar('\n');

#endif

    return &t[idx];
}



/********************************************************************************************************************
*   Documented in `mmu-funcs.h`
*///-----------------------------------------------------------------------------------------------------------------
Return_t cmn_MmuIsMapped(Addr_t a)
{
#if DEBUG_ENABLED(MmuIsMapped)

    SerialPutString("Checking if address ");
    SerialPutHex64(a);
    SerialPutString(" is mapped\n");

#endif

    INVLPG((Addr_t)GetPML4Entry(a));
    INVLPG((Addr_t)GetPDPTEntry(a));
    INVLPG((Addr_t)GetPDEntry(a));
    INVLPG((Addr_t)GetPTEntry(a));

#if DEBUG_ENABLED(MmuIsMapped)

    SerialPutString("Mapped? PML4 at ");
    SerialPutHex64((Addr_t)GetPML4Entry(a));
    SerialPutChar('\n');

#endif

    if (!GetPML4Entry(a)->p) return false;

#if DEBUG_ENABLED(MmuIsMapped)

    SerialPutString("mapped? PDPT at ");
    SerialPutHex64((Addr_t)GetPDPTEntry(a));
    SerialPutChar('\n');

#endif

    if (!GetPDPTEntry(a)->p) return false;

#if DEBUG_ENABLED(MmuIsMapped)

    SerialPutString("mapped? PD at ");
    SerialPutHex64((Addr_t)GetPDEntry(a));
    SerialPutChar('\n');

#endif

    if (!GetPDEntry(a)->p) return false;

#if DEBUG_ENABLED(MmuIsMapped)

    SerialPutString("mapped? PT at ");
    SerialPutHex64((Addr_t)GetPTEntry(a));
    SerialPutChar('\n');

#endif

    if (!GetPTEntry(a)->p) return false;

#if DEBUG_ENABLED(MmuIsMapped)

    SerialPutString("mapped.\n");

#endif

    return true;
}



/********************************************************************************************************************
*   Documented in `mmu-funcs.h`
*///-----------------------------------------------------------------------------------------------------------------
Return_t cmn_MmuUnmapPage(Addr_t a)
{
#if DEBUG_ENABLED(cmn_MmuUnmapPage)

    SerialPutString("In address space ");
    SerialPutHex64(GetAddressSpace());
    SerialPutString(", Unmapping page at address ");
    SerialPutHex64(a);
    SerialPutChar('\n');

#endif

    if (cmn_MmuIsMapped(a)) {
        *(uint64_t *)GetPTEntry(a) = 0;
        INVLPG(a);
    }

    return 0;
}



/********************************************************************************************************************
*   Documented in `mmu-funcs.h`
*///-----------------------------------------------------------------------------------------------------------------
Return_t cmn_MmuMapPage(Addr_t a, Frame_t f, int flags)
{
#if DEBUG_ENABLED(cmn_MmuMapPage)

    SerialPutString("In address space ");
    SerialPutHex64(GetAddressSpace());
    SerialPutString(", request was made to map address ");
    SerialPutHex64(a);
    SerialPutString(" to frame ");
    SerialPutHex64(f);
    SerialPutChar('\n');

    SerialPutString("Checking if the page is mapped\n");

#endif

    if (cmn_MmuIsMapped(a)) cmn_MmuUnmapPage(a);

#if DEBUG_ENABLED(cmn_MmuMapPage)

    SerialPutString(".. Done -- guaranteed unmapped\n");

#endif

    Frame_t t;
    PageEntry_t *ent = GetPML4Entry(a);

#if DEBUG_ENABLED(cmn_MmuMapPage)

    SerialPutString(".. Mapping PML4 @ ");
    SerialPutHex64((Addr_t)ent);
    SerialPutChar('\n');

#endif

    if (!ent->p) {
        t = PmmAlloc();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        WBNOINVD();
        INVLPG((Addr_t)GetPDPTEntry(a));

        uint64_t *tbl = (uint64_t *)((Addr_t)GetPDPTEntry(a) & 0xfffffffffffff000);
        for (int i = 0; i < 512; i ++) {
            tbl[i] = 0;
        }
    }

#if DEBUG_ENABLED(cmn_MmuMapPage)

    SerialPutString(".... [hex ");
    SerialPutHex64(*(uint64_t *)ent);
    SerialPutString("]\n");

#endif

    ent = GetPDPTEntry(a);

#if DEBUG_ENABLED(cmn_MmuMapPage)

    SerialPutString(".. Mapping PDPT @ ");
    SerialPutHex64((Addr_t)ent);
    SerialPutChar('\n');

#endif

    if (!ent->p) {
        t = PmmAlloc();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        WBNOINVD();
        INVLPG((Addr_t)GetPDEntry(a));

        uint64_t *tbl = (uint64_t *)((Addr_t)GetPDEntry(a) & 0xfffffffffffff000);
        for (int i = 0; i < 512; i ++) tbl[i] = 0;
    }

#if DEBUG_ENABLED(cmn_MmuMapPage)

    SerialPutString(".... [hex ");
    SerialPutHex64(*(uint64_t *)ent);
    SerialPutString("]\n");

#endif

    ent = GetPDEntry(a);

#if DEBUG_ENABLED(cmn_MmuMapPage)

    SerialPutString(".. Mapping PD @ ");
    SerialPutHex64((Addr_t)ent);
    SerialPutChar('\n');

#endif

    if (!ent->p) {
        t = PmmAlloc();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        WBNOINVD();
        INVLPG((Addr_t)GetPTEntry(a));

        uint64_t *tbl = (uint64_t *)((Addr_t)GetPTEntry(a) & 0xfffffffffffff000);
        for (int i = 0; i < 512; i ++) tbl[i] = 0;

#if DEBUG_ENABLED(cmn_MmuMapPage)

        SerialPutString("Address: ");
        SerialPutHex64((uint64_t)ent);
        SerialPutString(" .... [hex ");
        SerialPutHex64(*((uint64_t *)ent));
        SerialPutString("]\n");

#endif

    }

#if DEBUG_ENABLED(cmn_MmuMapPage)

    SerialPutString(".... [hex ");
    SerialPutHex64(*(uint64_t *)ent);
    SerialPutString("]\n");

#endif

    ent = GetPTEntry(a);

#if DEBUG_ENABLED(cmn_MmuMapPage)

    SerialPutString(".. Mapping PT @ ");
    SerialPutHex64((Addr_t)ent);
    SerialPutChar('\n');

#endif

    ent->frame = f;
    ent->rw = (flags&PG_WRT?1:0);
    ent->pcd = (flags&PG_DEV?1:0);
    ent->pwt = (flags&PG_DEV?1:0);
    ent->us = (flags&PG_DEV?1:0);
    ent->p = 1;

    WBNOINVD();
    INVLPG((Addr_t)a);

#if DEBUG_ENABLED(cmn_MmuMapPage)

    SerialPutString("Mapping complete!!\n");

#endif
    return 0;
}



