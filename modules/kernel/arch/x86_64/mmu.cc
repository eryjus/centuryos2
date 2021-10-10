//===================================================================================================================
//
//  mmu.cc -- Functions related to managing the Paging Tables
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
#include "kernel-funcs.h"
#include "printf.h"
#include "mmu.h"


extern "C" Addr_t GetCr3(void);


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

    return &t[idx];
}


PageEntry_t *GetPDPTEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)0xffffffffffe00000;
    uint64_t idx = (a >> (12 + (9 * 2))) & 0x3ffff;

    return &t[idx];
}


PageEntry_t *GetPDEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)0xffffffffc0000000;
    uint64_t idx = (a >> (12 + (9 * 1))) & 0x7ffffff;

    return &t[idx];
}


PageEntry_t *GetPTEntry(Addr_t a)
{
    PageEntry_t *t = (PageEntry_t *)0xffffff8000000000;
    uint64_t idx = (a >> (12 + (9 * 0))) & 0xfffffffff;

    return &t[idx];
}


//
// -- Some assembly CPU instructions
//    ------------------------------
static inline void INVLPG(Addr_t a) { __asm volatile("invlpg (%0)" :: "r"(a) : "memory"); }


//
// -- Check if an address is mapped
//    -----------------------------
bool krn_MmuIsMapped(int, Addr_t a)
{
//    kprintf("Checking mapping (reporting mapped only)\n");

    INVLPG((Addr_t)GetPML4Entry(a));
    INVLPG((Addr_t)GetPDPTEntry(a));
    INVLPG((Addr_t)GetPDEntry(a));
    INVLPG((Addr_t)GetPTEntry(a));

    if (!GetPML4Entry(a)->p) return false;
    if (!GetPDPTEntry(a)->p) return false;
    if (!GetPDEntry(a)->p) return false;
    if (!GetPTEntry(a)->p) return false;

//    kprintf("... mapped\n");
    return true;
}


//
// -- Safely Unmap a page
//    -------------------
int krn_MmuUnmapPage(int, Addr_t a)
{
    kprintf("In address space %p, Unmapping page at address %p\n", GetCr3(), a);

    if (krn_MmuIsMapped(0, a)) {
//        kprintf("Unmapping address %p..\n", GetPTEntry(a));
        *(uint64_t *)GetPTEntry(a) = 0;
    }

    INVLPG((Addr_t)GetPTEntry(a));
    INVLPG(a);

//    kprintf("!! Unmapped\n");

    return 0;
}


//
// -- Map a page to a frame
//    ---------------------
int krn_MmuMapPage(int, Addr_t a, Frame_t f, int flags)
{
    a &= ~(PAGE_SIZE - 1);
    kprintf("In address space %p, request was made to map address %p to frame %p\n", GetCr3(), a, f);
//    kprintf(".. PML4 address is %p\n", GetPML4Entry(a));
//    kprintf(".. PDPT address is %p\n", GetPDPTEntry(a));
//    kprintf("..   PD address is %p\n", GetPDEntry(a));
//    kprintf("..   PT address is %p\n", GetPTEntry(a));

    if (!a || !f) return -EINVAL;
    if (krn_MmuIsMapped(0, a)) {
        kprintf("!!! CHECK THE CODE!!! The page is already mapped and will be unmapped!\n");
        krn_MmuUnmapPage(0, a);
    }

//    kprintf(".. The page is guaranteed unmapped\n");

    Frame_t t;
    PageEntry_t *ent = GetPML4Entry(a);
//    kprintf(".. PML4 address is still %p (contents %p)?\n", ent, *(uint64_t *)ent);

    if (!ent->p) {
        t = PmmAlloc();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        __asm volatile ("wbnoinvd" ::: "memory");
        INVLPG((Addr_t)GetPDPTEntry(a));

        uint64_t *tbl = (uint64_t *)((Addr_t)GetPDPTEntry(a) & 0xfffffffffffff000);
//        kprintf(".... Clearing new table from address %p\n", tbl);
        for (int i = 0; i < 512; i ++) {
            tbl[i] = 0;
        }

//        kprintf(".... PML4 address is now %p (contents %p)?\n", ent, *(uint64_t *)ent);
    }

    ent = GetPDPTEntry(a);
//    kprintf(".. PDPT address is still %p (contents %p)?\n", ent, *(uint64_t *)ent);

    if (!ent->p) {
        t = PmmAlloc();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        __asm volatile ("wbnoinvd" ::: "memory");
        INVLPG((Addr_t)GetPDEntry(a));

        uint64_t *tbl = (uint64_t *)((Addr_t)GetPDEntry(a) & 0xfffffffffffff000);
//        kprintf(".... Clearing new table from address %p\n", tbl);
        for (int i = 0; i < 512; i ++) {
            tbl[i] = 0;
        }

//        kprintf(".... PDPT address is now %p (contents %p)?\n", ent, *(uint64_t *)ent);
    }

    ent = GetPDEntry(a);
//    kprintf("..   PD address is still %p (contents %p)?\n", ent, *(uint64_t *)ent);

    if (!ent->p) {
        t = PmmAlloc();
        ent->frame = t;
        ent->rw = 1;
        ent->p = 1;

        __asm volatile ("wbnoinvd" ::: "memory");
        INVLPG((Addr_t)GetPTEntry(a));

        uint64_t *tbl = (uint64_t *)((Addr_t)GetPTEntry(a) & 0xfffffffffffff000);
//        kprintf(".... Clearing new table from address %p\n", tbl);
        for (int i = 0; i < 512; i ++) {
            tbl[i] = 0;
        }

//        kprintf(".... PD address is now %p (contents %p)?\n", ent, *(uint64_t *)ent);
    }

    ent = GetPTEntry(a);
//    kprintf("..   PT address is still %p (contents %p)?\n", ent, *(uint64_t *)ent);

    ent->frame = f;
    ent->rw = (flags&PG_WRT?1:0);
    ent->pcd = (flags&PG_DEV?1:0);
    ent->pwt = (flags&PG_DEV?1:0);
    ent->us = (flags&PG_DEV?1:0);
    ent->p = 1;

    __asm volatile ("wbnoinvd" ::: "memory");
    INVLPG((Addr_t)a);

//    kprintf("!! Mapping complete\n");

    return 0;
}



//
// -- Map a page in another address space
//    -----------------------------------
int krn_MmuMapPageEx(int, Addr_t space, Addr_t a, Frame_t f, int flags)
{
    kprintf("Preparing to map a page in another address space\n");
    Addr_t cr3 = GetAddressSpace();
    LoadCr3(space);
    int rv = krn_MmuMapPage(0, a, f, flags);

    LoadCr3(cr3);
    kprintf("..done\n");

    return rv;
}


//
// -- Unmap a page in another address space
//    -------------------------------------
int krn_MmuUnmapEx(int, Addr_t space, Addr_t a)
{
    Addr_t cr3 = GetAddressSpace();
    LoadCr3(space);

    int rv = krn_MmuUnmapPage(0, a);

    LoadCr3(cr3);

    return rv;
}


#ifdef kprintf
#undef kprintf
extern "C" int kprintf(const char *, ...);
#endif

//
// -- Dump the MMU Tables for a specific address
//    ------------------------------------------
int krn_MmuDump(int, Addr_t addr)
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


