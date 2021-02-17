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
#include "internal.h"
#include "mmu.h"
#include "elf.h"
#include "idt.h"
#include "modules.h"


//
// -- This is the structure that will be available to determine how to load a module
//    ------------------------------------------------------------------------------
typedef struct Module_t {
    char sig[16];
    char name[16];
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
typedef int (*EarlyInit_t)(BootInterface_t *);



//
// -- some additional declarations
//    ----------------------------
extern "C" Addr_t LoadCr3(Addr_t);
extern BootInterface_t *loaderInterface;


//
// -- we need to keep track (temporarily) of the `cr3` values for each module
//    -----------------------------------------------------------------------
typedef struct ModuleInternal_t {
    Addr_t entries;
    Frame_t cr3Frame;
    bool loaded;
} ModuleInternal_t;

ModuleInternal_t modInternal[MAX_MODS];


//
// -- Check the module for validity
//    -----------------------------
Module_t *ModuleCheck(Addr_t addr)
{
    Module_t *rv = (Module_t *)addr;

    kprintf("Checking Module for validity\n");

    if (rv->sig[0]  != 'C') return NULL;
    if (rv->sig[1]  != 'e') return NULL;
    if (rv->sig[2]  != 'n') return NULL;
    if (rv->sig[3]  != 't') return NULL;
    if (rv->sig[4]  != 'u') return NULL;
    if (rv->sig[5]  != 'r') return NULL;
    if (rv->sig[6]  != 'y') return NULL;
    if (rv->sig[7]  != ' ') return NULL;
    if (rv->sig[8]  != 'O') return NULL;
    if (rv->sig[9]  != 'S') return NULL;
    if (rv->sig[10] != ' ') return NULL;
    if (rv->sig[11] != '6') return NULL;
    if (rv->sig[12] != '4') return NULL;
    if (rv->sig[13] !=  0 ) return NULL;
    if (rv->sig[14] !=  0 ) return NULL;
    if (rv->sig[15] !=  0 ) return NULL;

    if (rv->earlyInit == 0) return NULL;
    if (rv->intCnt + rv->internalCnt + rv->osCnt == 0) return NULL;

    // -- we have a good module
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

        // -- create a new page table structure
        modInternal[i].cr3Frame = PmmAlloc();
        krn_MmuMapPage(modInternal[i].cr3Frame << 12, modInternal[i].cr3Frame, true);

        uint64_t *t = (uint64_t *)(modInternal[i].cr3Frame << 12);
        for (int j = 0; j < 512; j ++) {
            if ((j >= 0x100 && j < 0x140) || (j >= 0x1f0 && j < 0x1ff)) {   // -- kernel and stacks
                t[j] = cr3[j];
            } else t[j]  = 0;
        }

        t[511] = ((uint64_t)t) | 0x003;

#if 0
        kprintf("Comparing PML4 at %p (%p)\n", t, cr3[511]);
        for (int x = 0; x < 512; x ++) {
            if (t[x] != 0) kprintf("%x > %p (%p)\n", x, t[x], cr3[x]);
        }
#endif

        MmuUnmapPage(modInternal[i].cr3Frame << 12);

        kprintf("Loading new CR3 at frame %p\n", modInternal[i].cr3Frame);
        Addr_t oldCr3 = LoadCr3(modInternal[i].cr3Frame << 12);


        // -- Load the ELF image into the new CR3
        Addr_t moduleAddr = loaderInterface->modAddr[i];
        kprintf("Loading Module located at %p\n", moduleAddr);

        kprintf(".. Old CR3: %p; New: %p\n", oldCr3, modInternal[i].cr3Frame << 12);
        krn_MmuMapPage(moduleAddr, moduleAddr >> 12, false);
        kprintf(".. Preparing to map the loaderInterface struct\n");
        kprintf(".. Image header mapped\n");
        modInternal[i].entries = ElfLoadImage(moduleAddr);


        // -- Now, check the module
        Module_t *mod = ModuleCheck(modInternal[i].entries);
        if (mod) {
            EarlyInit_t init = (EarlyInit_t)mod->earlyInit;
            int res = init(loaderInterface);
            modInternal[i].loaded = true;

            // -- if we are not to load the module (non-zero return) then set the name to null
            if (res) {
                modInternal[i].loaded = false;
            }
        } else {
            modInternal[i].loaded = false;
        }


        if (modInternal[i].loaded) {
            // -- Now install the hooks
            int h;
            for (h = 0; h < mod->intCnt; h ++) {
                IdtSetHandler(mod->hooks[h].loc, 8, (IdtHandlerFunc_t *)mod->hooks[h].target, 0, 0);
            }

            for ( ; h < mod->intCnt + mod->internalCnt; h ++) {
                kprintf(".... Hooking Internal Function %d: %p from %p\n", mod->hooks[h].loc, mod->hooks[h].target, modInternal[i].cr3Frame);
                SetInternalHandler(mod->hooks[h].loc, mod->hooks[h].target, modInternal[i].cr3Frame);
            }

            for ( ; h < mod->intCnt + mod->internalCnt + mod->osCnt; h ++) {
                SetInternalService(mod->hooks[h].loc, mod->hooks[h].target, modInternal[i].cr3Frame);
            }
        } else {
            // -- unload the module
        }


        // -- Clean up from loading the module
        kprintf(".. The entry point is at %p\n", modInternal[i].entries);
        MmuUnmapPage(moduleAddr);
        LoadCr3(oldCr3);
    }
}


