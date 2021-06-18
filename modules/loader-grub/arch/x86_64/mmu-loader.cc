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


//#define USE_SERIAL

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
    uint64_t idx = (a >> 39) & 0x1ff;
//    SerialPutString("PML4 idx = ");
//    SerialPutHex64(idx);
//    SerialPutChar('\n');
    return &t[idx];
}


PageEntry_t *GetPDPTEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)0xffffffffffe00000;
    uint64_t idx = (a >> 30) & 0x3ffff;
//    SerialPutString("PDPT idx = ");
//    SerialPutHex64(idx);
//    SerialPutChar('\n');
    return &t[idx];
}


PageEntry_t *GetPDEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)0xffffffffc0000000;
    uint64_t idx = (a >> 21) & 0x7ffffff;
//    SerialPutString("PD idx = ");
//    SerialPutHex64(idx);
//    SerialPutChar('\n');
    return &t[idx];
}


PageEntry_t *GetPTEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)0xffffff8000000000;
    uint64_t idx = (a >> 12) & 0xfffffffff;
//    SerialPutString("PT idx = ");
//    SerialPutHex64(idx);
//    SerialPutChar('\n');
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
void MmuMapPage(Addr_t a, Frame_t f, int flags) { ldr_MmuMapPage(a, f, flags); }
void MmuUnmapPage(Addr_t a) { ldr_MmuUnmapPage(a); }


//
// -- Check if an address is mapped
//    -----------------------------
bool MmuIsMapped(Addr_t a)
{
    SerialPutString("Checking if address ");
    SerialPutHex64(a);
    SerialPutString(" is mapped\n");

    INVLPG((Addr_t)GetPML4Entry(a));
    INVLPG((Addr_t)GetPDPTEntry(a));
    INVLPG((Addr_t)GetPDEntry(a));
    INVLPG((Addr_t)GetPTEntry(a));

    SerialPutString("Mapped? PML4 at ");
    SerialPutHex64((Addr_t)GetPML4Entry(a));
    SerialPutChar('\n');

    if (!GetPML4Entry(a)->p) return false;

    SerialPutString("mapped? PDPT at ");
    SerialPutHex64((Addr_t)GetPDPTEntry(a));
    SerialPutChar('\n');

    if (!GetPDPTEntry(a)->p) return false;

    SerialPutString("mapped? PD at ");
    SerialPutHex64((Addr_t)GetPDEntry(a));
    SerialPutChar('\n');

    if (!GetPDEntry(a)->p) return false;

    SerialPutString("mapped? PT at ");
    SerialPutHex64((Addr_t)GetPTEntry(a));
    SerialPutChar('\n');

    if (!GetPTEntry(a)->p) return false;

    SerialPutString("mapped.\n");

    return true;
}


//
// -- Safely Unmap a page
//    -------------------
void ldr_MmuUnmapPage(Addr_t a)
{
    SerialPutString("Unmapping ");
    SerialPutHex64(a);
    SerialPutChar('\n');

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
    SerialPutString("Mapping ");
    SerialPutHex64(a);
    SerialPutString(" to frame ");
    SerialPutHex64(f);
    SerialPutChar('\n');

    SerialPutString("Checking if the page is mapped\n");
    if (MmuIsMapped(a)) ldr_MmuUnmapPage(a);
    SerialPutString(".. Done -- guaranteed unmapped\n");


    Frame_t t;
    PageEntry_t *ent = GetPML4Entry(a);
    INVLPG((Addr_t)ent);
    SerialPutString(".. Mapping PML4 @ ");
    SerialPutHex64((Addr_t)ent);
    SerialPutChar('\n');
    if (!ent->p) {
        t = MmuGetTable();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        INVLPG((Addr_t)GetPDPTEntry(a));

//        uint64_t *tbl = (uint64_t *)GetPDPTEntry(a & 0xfffffffffffff000);
        uint64_t *tbl = (uint64_t *)((Addr_t)GetPDPTEntry(a) & 0xfffffffffffff000);
        for (int i = 0; i < 512; i ++) {
            tbl[i] = 0;
        }
    }

    SerialPutString(".... [hex ");
    SerialPutHex64(*(uint64_t *)ent);
    SerialPutString("]\n");

    ent = GetPDPTEntry(a);
    INVLPG((Addr_t)ent);
    SerialPutString(".. Mapping PDPT @ ");
    SerialPutHex64((Addr_t)ent);
    SerialPutChar('\n');
    if (!ent->p) {
        t = MmuGetTable();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        INVLPG((Addr_t)GetPDEntry(a));

//        uint64_t *tbl = (uint64_t *)GetPDEntry(a & 0xffffffffff000000);
        uint64_t *tbl = (uint64_t *)((Addr_t)GetPDEntry(a) & 0xfffffffffffff000);
        for (int i = 0; i < 512; i ++) tbl[i] = 0;
    }

    SerialPutString(".... [hex ");
    SerialPutHex64(*(uint64_t *)ent);
    SerialPutString("]\n");

    ent = GetPDEntry(a);
    INVLPG((Addr_t)ent);
    SerialPutString(".. Mapping PD @ ");
    SerialPutHex64((Addr_t)ent);
    SerialPutChar('\n');
    if (!ent->p) {
        t = MmuGetTable();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        INVLPG((Addr_t)GetPTEntry(a));

//        uint64_t *tbl = (uint64_t *)GetPTEntry(a & 0xfffffff000000000);
        uint64_t *tbl = (uint64_t *)((Addr_t)GetPTEntry(a) & 0xfffffffffffff000);
        for (int i = 0; i < 512; i ++) tbl[i] = 0;

        SerialPutString("Address: ");
        SerialPutHex64((uint64_t)ent);
        SerialPutString(" .... [hex ");
        SerialPutHex64(*((uint64_t *)ent));
        SerialPutString("]\n");
    }

    SerialPutString(".... [hex ");
    SerialPutHex64(*(uint64_t *)ent);
    SerialPutString("]\n");

    ent = GetPTEntry(a);
    INVLPG((Addr_t)ent);
    SerialPutString(".. Mapping PT @ ");
    SerialPutHex64((Addr_t)ent);
    SerialPutChar('\n');
    ent->frame = f;
    ent->rw = (flags&PG_WRT?1:0);
    ent->p = 1;
}

