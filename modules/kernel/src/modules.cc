//===================================================================================================================
//
//  modules.cc -- Handle the initialization of the modules
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-02  Initial  v0.0.4   ADCL  Initial version
//
//===================================================================================================================


//#define USE_SERIAL

#include "types.h"
#include "printf.h"
#include "boot-interface.h"
#include "kernel-funcs.h"
#include "mmu.h"
#include "elf.h"
#include "modules.h"


extern BootInterface_t *loaderInterface;//
// -- This is the structure that will be available to determine how to load a module
//    ------------------------------------------------------------------------------
typedef struct Module_t {
    char sig[16];
    char name[16];
    uint64_t initSeq;
    uint64_t earlyInit;
    uint64_t lateInit;
    uint64_t intCnt;
    uint64_t internalCnt;
    uint64_t osCnt;
    struct {
        uint64_t loc;
        uint64_t target;
    } hooks [0];
} Module_t;


//
// -- the function prototype for early init calls
//    -------------------------------------------
typedef int (*EarlyInit_t)(void);



//
// -- some additional prototypes
//    --------------------------
extern "C" Addr_t LoadCr3(Addr_t);


//
// -- we need to keep track (temporarily) of the `cr3` values for each module
//    -----------------------------------------------------------------------
Frame_t cr3Frame[MAX_MODS] = { 0 };
Addr_t entries[MAX_MODS] = { 0 };


//
// -- Check the module for validity
//    -----------------------------
Module_t *ModuleCheck(Addr_t addr)
{
    Module_t *rv = (Module_t *)addr;

    kprintf("Checking Module for validity\n");

    return rv;
}


//
// -- Find and perform the early initialization for each module
//    ---------------------------------------------------------
void ModuleEarlyInit()
{
    uint64_t *cr3 = (uint64_t *)0xfffffffffffff000;

    kprintf("Checking recursive mapping %p\n", cr3[511]);
    kprintf("Checking kernel mapping %p\n", cr3[0x100]);

    for (int i = 0; i < loaderInterface->modCount; i ++) {
        kprintf("Module %d located at %p\n", i, loaderInterface->modAddr[i]);

        cr3Frame[i] = PmmAlloc();
        MmuMapPage(cr3Frame[i] << 12, cr3Frame[i], true);

        uint64_t *t = (uint64_t *)(cr3Frame[i] << 12);
        for (int j = 0; j < 512; j ++) {
            if ((j >= 0x100 && j < 0x140) || (j >= 0x1f0 && j < 0x1ff)) {
                t[j] = cr3[j];
            } else t[j]  = 0;
        }

        t[511] = ((Addr_t)t) | 3;

        kprintf("Loading new CR3 at %p\n", cr3Frame[i]);
        Addr_t moduleAddr = loaderInterface->modAddr[i];
        Addr_t oldCr3 = LoadCr3(cr3Frame[i] << 12);

        kprintf(".. Old CR3: %p; New: %p\n", oldCr3, cr3Frame[i] << 12);
        MmuMapPage(moduleAddr, moduleAddr >> 12, false);
        kprintf(".. Image header mapped\n");
        entries[i] = ElfLoadImage(moduleAddr);

        Module_t *mod = ModuleCheck(entries[i]);
        if (mod) {
            EarlyInit_t init = (EarlyInit_t)mod->earlyInit;
            int res = init();

            // -- if we are not to load the module (non-zero return) then set the name to null
            if (res) {
                mod->name[0] = 0;
            }
        }

        kprintf(".. The entry point is at %p\n", entries[i]);
        LoadCr3(oldCr3);

        MmuUnmapPage(cr3Frame[i] << 12);
    }
}


