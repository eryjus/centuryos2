//===================================================================================================================
//
//  mmu-loader.cc -- Functions related to managing the Paging Tables
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-03  Initial  v0.0.1   ADCL  Initial version
//
//===================================================================================================================


#define USE_SERIAL

#include "types.h"
#include "serial.h"
#include "mmu.h"


#define PG_NONE     (0)
#define PG_WRT      (1<<0)
#define PG_KRN      (1<<1)
#define PG_DEV      (1<<15)



//
// -- This is a 64-bit page entry for all levels of the page tables
//    -------------------------------------------------------------
typedef struct PageEntry_t {
    unsigned int p : 1;                 // Is the page present?
    unsigned int rw : 1;                // set to 1 to allow writes
    unsigned int us : 1;                // 0=Supervisor; 1=user
    unsigned int pwt : 1;               // Page Write Through
    unsigned int pcd : 1;               // Page-level cache disable
    unsigned int a : 1;                 // accessed
    unsigned int d : 1;                 // dirty (needs to be written for a swap)
    unsigned int pat : 1;               // set to 0 for tables, page Page Attribute Table (set to 0)
    unsigned int g : 1;                 // Global (set to 0)
    unsigned int k : 1;                 // Is this a kernel page?
    unsigned int avl : 2;               // Available for software use
    Frame_t frame : 36;                 // This is the 4K aligned page frame address (or table address)
    unsigned int reserved : 4;          // reserved bits
    unsigned int software : 11;         // software use bits
    unsigned int xd : 1;                // execute disable
} __attribute__((packed)) PageEntry_t;


//
// -- some inline functions to handle access to Page Table Entries
//    ------------------------------------------------------------
PageEntry_t *GetPML4Entry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)0xfffffffffffff000;
    uint64_t idx = (a >> (12 + (9 * 3))) & 0x1ff;

#if DEBUG_ENABLED(GetPML4Entry)

    SerialPutString("PML4 idx = ");
    SerialPutHex64(idx);
    SerialPutChar('\n');

#endif

    return &t[idx];
}


PageEntry_t *GetPDPTEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)0xffffffffffe00000;
    uint64_t idx = (a >> (12 + (9 * 2))) & 0x3ffff;

#if DEBUG_ENABLED(GetPDPTEntry)

    SerialPutString("PDPT idx = ");
    SerialPutHex64(idx);
    SerialPutChar('\n');

#endif

    return &t[idx];
}


PageEntry_t *GetPDEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)0xffffffffc0000000;
    uint64_t idx = (a >> (12 + (9 * 1))) & 0x7ffffff;

#if DEBUG_ENABLED(GetPDEntry)

    SerialPutString("PD idx = ");
    SerialPutHex64(idx);
    SerialPutChar('\n');

#endif

    return &t[idx];
}


PageEntry_t *GetPTEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)0xffffff8000000000;
    uint64_t idx = (a >> (12 + (9 * 0))) & 0xfffffffff;

#if DEBUG_ENABLED(GetPTEntry)

    SerialPutString("PT idx = ");
    SerialPutHex64(idx);
    SerialPutChar('\n');

#endif

    return &t[idx];
}


//
// -- Some assembly CPU instructions
//    ------------------------------
static inline void INVLPG(Addr_t a) { __asm volatile("invlpg (%0)" :: "r"(a) : "memory"); }


//
// -- allocate a new frame
//    --------------------
Frame_t MmuGetTable(void)
{
    extern Frame_t earlyFrame;
    return earlyFrame ++;
}


//
// -- Some wrapper functions
//    ----------------------
//void MmuMapPage(Addr_t a, Frame_t f, int flags) { ldr_MmuMapPage(a, f, flags); }
//void MmuUnmapPage(Addr_t a) { ldr_MmuUnmapPage(a); }


//
// -- Check if an address is mapped
//    -----------------------------
bool MmuIsMapped(Addr_t a)
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


//
// -- Safely Unmap a page
//    -------------------
void ldr_MmuUnmapPage(Addr_t a)
{
#if DEBUG_ENABLED(ldr_MmuUnmapPage)

    SerialPutString("In address space ");
    SerialPutHex64(GetCr3());
    SerialPutString(", Unmapping page at address ");
    SerialPutHex64(a);
    SerialPutChar('\n');

#endif

    if (MmuIsMapped(a)) {
        *(uint64_t *)GetPTEntry(a) = 0;
        INVLPG(a);
    }
}


//
// -- Map a page to a frame
//    ---------------------
void ldr_MmuMapPage(Addr_t a, Frame_t f, int flags)
{
#if DEBUG_ENABLED(ldr_MmuMapPage)

    SerialPutString("In address space ");
    SerialPutHex64(GetCr3());
    SerialPutString(", request was made to map address ");
    SerialPutHex64(a);
    SerialPutString(" to frame ");
    SerialPutHex64(f);
    SerialPutChar('\n');

    SerialPutString("Checking if the page is mapped\n");

#endif

    if (MmuIsMapped(a)) ldr_MmuUnmapPage(a);

#if DEBUG_ENABLED(ldr_MmuMapPage)

    SerialPutString(".. Done -- guaranteed unmapped\n");

#endif

    Frame_t t;
    PageEntry_t *ent = GetPML4Entry(a);

#if DEBUG_ENABLED(ldr_MmuMapPage)

    SerialPutString(".. Mapping PML4 @ ");
    SerialPutHex64((Addr_t)ent);
    SerialPutChar('\n');

#endif

    if (!ent->p) {
        t = MmuGetTable();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        __asm volatile ("wbnoinvd" ::: "memory");
        INVLPG((Addr_t)GetPDPTEntry(a));

        uint64_t *tbl = (uint64_t *)((Addr_t)GetPDPTEntry(a) & 0xfffffffffffff000);
        for (int i = 0; i < 512; i ++) {
            tbl[i] = 0;
        }
    }

#if DEBUG_ENABLED(ldr_MmuMapPage)

    SerialPutString(".... [hex ");
    SerialPutHex64(*(uint64_t *)ent);
    SerialPutString("]\n");

#endif

    ent = GetPDPTEntry(a);

#if DEBUG_ENABLED(ldr_MmuMapPage)

    SerialPutString(".. Mapping PDPT @ ");
    SerialPutHex64((Addr_t)ent);
    SerialPutChar('\n');

#endif

    if (!ent->p) {
        t = MmuGetTable();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        __asm volatile ("wbnoinvd" ::: "memory");
        INVLPG((Addr_t)GetPDEntry(a));

        uint64_t *tbl = (uint64_t *)((Addr_t)GetPDEntry(a) & 0xfffffffffffff000);
        for (int i = 0; i < 512; i ++) tbl[i] = 0;
    }

#if DEBUG_ENABLED(ldr_MmuMapPage)

    SerialPutString(".... [hex ");
    SerialPutHex64(*(uint64_t *)ent);
    SerialPutString("]\n");

#endif

    ent = GetPDEntry(a);

#if DEBUG_ENABLED(ldr_MmuMapPage)

    SerialPutString(".. Mapping PD @ ");
    SerialPutHex64((Addr_t)ent);
    SerialPutChar('\n');

#endif

    if (!ent->p) {
        t = MmuGetTable();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        __asm volatile ("wbnoinvd" ::: "memory");
        INVLPG((Addr_t)GetPTEntry(a));

        uint64_t *tbl = (uint64_t *)((Addr_t)GetPTEntry(a) & 0xfffffffffffff000);
        for (int i = 0; i < 512; i ++) tbl[i] = 0;

#if DEBUG_ENABLED(ldr_MmuMapPage)

        SerialPutString("Address: ");
        SerialPutHex64((uint64_t)ent);
        SerialPutString(" .... [hex ");
        SerialPutHex64(*((uint64_t *)ent));
        SerialPutString("]\n");

#endif

    }

#if DEBUG_ENABLED(ldr_MmuMapPage)

    SerialPutString(".... [hex ");
    SerialPutHex64(*(uint64_t *)ent);
    SerialPutString("]\n");

#endif

    ent = GetPTEntry(a);

#if DEBUG_ENABLED(ldr_MmuMapPage)

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

    __asm volatile ("wbnoinvd" ::: "memory");
    INVLPG((Addr_t)a);

#if DEBUG_ENABLED(ldr_MmuMapPage)

    SerialPutString("Mapping complete!!\n");

#endif
}



//
// -- Create an empty PDPT Table
//    --------------------------
void MmuEmptyPdpt(int index)
{
    extern Frame_t earlyFrame;

#if DEBUG_ENABLED(MmuEmptyPdpt)

        SerialPutString("Creating a PDPT table for PML4 index: ");
        SerialPutHex64(index);
        SerialPutChar('\n');

#endif

    PageEntry_t *ent = &((PageEntry_t *)0xfffffffffffff000)[index];

    if (!ent->p) {
        ent->frame = earlyFrame ++;

#if DEBUG_ENABLED(MmuEmptyPdpt)

        SerialPutString("Next frame is: ");
        SerialPutHex64(ent->frame);
        SerialPutChar('\n');

#endif

        ent->rw = 1;
        ent->p = 1;

        uint64_t *tbl = (uint64_t *)(0xffffffffffe00000 | (index << 12));
        INVLPG((Addr_t)tbl);

#if DEBUG_ENABLED(MmuEmptyPdpt)

        SerialPutString("Clearing the table starting at: ");
        SerialPutHex64((Addr_t)tbl);
        SerialPutChar('\n');

#endif

        for (int i = 0; i < 512; i ++) {
#if DEBUG_ENABLED(MmuEmptyPdpt)
            SerialPutString("At address: ");
            SerialPutHex64((Addr_t)&tbl[i]);
            SerialPutChar('\n');

#endif
            tbl[i] = 0;
        }
    }
}