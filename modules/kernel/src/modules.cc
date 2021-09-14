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


#define USE_SERIAL

#include "types.h"
#include "printf.h"
#include "boot-interface.h"
#include "kernel-funcs.h"
#include "internals.h"
#include "mmu.h"
#include "elf.h"
#include "idt.h"
#include "modules.h"


//
// -- The module handler stack size
//    -----------------------------
#define MODULE_STACK_SIZE   0x1000

//
// -- This is the structure that will be available to determine how to load a module
//    ------------------------------------------------------------------------------
typedef struct Module_t {
    char sig[16];
    char name[16];
    uint64_t earlyInit;
    uint64_t lateInit;
    uint64_t stacksStart;
    uint64_t intCnt;
    uint64_t internalCnt;
    uint64_t osCnt;
    struct {
        uint64_t loc;
        uint64_t target;
        uint64_t stack;
    } hooks [0];
} Module_t;


//
// -- the function prototype for early init calls
//    -------------------------------------------
typedef int (*EarlyInit_t)(BootInterface_t *);
typedef void (*LateInit_t)(void);



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
    Frame_t cr3Addr;
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

    kprintf(".. checking earlyInit function\n");
    if (rv->earlyInit == 0) return NULL;

    kprintf(".. checking published feature count\n");
    if (rv->intCnt + rv->internalCnt + rv->osCnt == 0) return NULL;

    kprintf(".. valid!\n");

    // -- we have a good module
    return rv;
}


//
// -- Find and perform the early initialization for each module
//    ---------------------------------------------------------
void ModuleEarlyInit()
{
    uint64_t *cr3 = (uint64_t *)0xfffffffffffff000;
    uint64_t currentStack;

//    kprintf("Checking recursive mapping %p\n", cr3[511]);
//    kprintf("Checking kernel mapping %p\n", cr3[0x100]);

    for (int i = 0; i < loaderInterface->modCount; i ++) {
//        kprintf("Module %d located at %p\n", i, loaderInterface->modAddr[i]);

        // -- create a new page table structure
        Frame_t cr3Frame = PmmAlloc();
        modInternal[i].cr3Addr = cr3Frame << 12;

//        kprintf("Preparing to map address %p to frame %p\n", modInternal[i].cr3Addr, cr3Frame);

        MmuMapPage(modInternal[i].cr3Addr, cr3Frame, PG_WRT);       // This is identity mapped!

        kprintf("Mapping the module\n");

        uint64_t *t = (uint64_t *)(modInternal[i].cr3Addr);
        for (int j = 0; j < 512; j ++) {
            if ((j >= 0x100 && j < 0x140) || (j >= 0x1f0 && j < 0x1ff)) {   // -- kernel and stacks
                t[j] = cr3[j];
            } else t[j]  = 0;
        }

        t[511] = ((uint64_t)t) | 0x003;

        MmuUnmapPage(modInternal[i].cr3Addr);

//        kprintf("Loading new CR3 at frame %p (Addr %p)\n", cr3Frame, modInternal[i].cr3Addr);     // <-- this line fixes the problem
        Addr_t oldCr3 = LoadCr3(modInternal[i].cr3Addr);


        // -- Load the ELF image into the new CR3
        Addr_t moduleAddr = loaderInterface->modAddr[i];
        kprintf("Loading Module located at %p\n", moduleAddr);
        kprintf(".. Old CR3: %p; New: %p\n", oldCr3, modInternal[i].cr3Addr);
        kprintf(".. Module Address is getting mapped to %p\n", moduleAddr);

        kprintf(".. Image header mapped\n");
        modInternal[i].entries = ElfLoadImage(moduleAddr);
        kprintf(".. Elf Loaded\n");


        // -- Now, check the module
        Module_t *mod = ModuleCheck(modInternal[i].entries);
        if (mod) {
            kprintf(".. module name is %s\n", mod->name);
            EarlyInit_t init = (EarlyInit_t)mod->earlyInit;
            kprintf(".. calling early init function at %p\n", (Addr_t)init);
            int res = init(loaderInterface);
            kprintf(".. early init completed\n");

            modInternal[i].loaded = true;

            // -- if we are not to load the module (non-zero return) then set the name to null
            if (res) {
                modInternal[i].loaded = false;
            }
        } else {
            modInternal[i].loaded = false;
        }


        if (modInternal[i].loaded) {
            currentStack = mod->stacksStart;

            kprintf(".. Hooking services: %d interrupts; %d internal functions; %d OS services\n", mod->intCnt, mod->internalCnt, mod->osCnt);

            // -- Now install the hooks
            unsigned long h;
            for (h = 0; h < mod->intCnt; h ++) {
                kprintf(".... Hooking Interrupt Vector %d: %p from %p\n", mod->hooks[h].loc, mod->hooks[h].target, modInternal[i].cr3Addr);
                kprintf("...... stack at %p\n", currentStack);

                if (currentStack) {
                    krn_MmuMapPage(0, currentStack, PmmAlloc(), PG_WRT);
                    currentStack += MODULE_STACK_SIZE;
                }

                krn_SetVectorHandler(0, mod->hooks[h].loc, mod->hooks[h].target, modInternal[i].cr3Addr, currentStack);
            }

            for ( ; h < mod->intCnt + mod->internalCnt; h ++) {
                kprintf(".... Hooking Internal Function %d: %p from %p\n", mod->hooks[h].loc, mod->hooks[h].target, modInternal[i].cr3Addr);
                kprintf("...... stack at %p\n", currentStack);

                if (currentStack) {
                    krn_MmuMapPage(0, currentStack, PmmAlloc(), PG_WRT);
                    currentStack += MODULE_STACK_SIZE;
                }

                krn_SetInternalHandler(0, mod->hooks[h].loc, mod->hooks[h].target, modInternal[i].cr3Addr, currentStack);
            }

            for ( ; h < mod->intCnt + mod->internalCnt + mod->osCnt; h ++) {
                kprintf(".... Hooking OS Service %d: %p from %p\n", mod->hooks[h].loc, mod->hooks[h].target, modInternal[i].cr3Addr);
                kprintf("...... stack at %p\n", currentStack);

                if (currentStack) {
                    krn_MmuMapPage(0, currentStack, PmmAlloc(), PG_WRT);
                    currentStack += MODULE_STACK_SIZE;
                }

                krn_SetServiceHandler(0, mod->hooks[h].loc, mod->hooks[h].target, modInternal[i].cr3Addr, currentStack);
            }
        } else {
            // -- unload the module
        }


        // -- Clean up from loading the module
        LoadCr3(oldCr3);
    }
}


//
// -- Call the Late Initialization functions for any loaded module
//    ------------------------------------------------------------
void ModuleLateInit(void)
{
    for (int i = 0; i < loaderInterface->modCount; i ++) {
        kprintf("Checking module %d\n", i);
        if (modInternal[i].loaded) {
            Addr_t oldCr3 = LoadCr3(modInternal[i].cr3Addr);
            Addr_t moduleAddr = loaderInterface->modAddr[i];
            MmuMapPage(moduleAddr, moduleAddr >> 12, PG_NONE);
            Module_t *mod = ModuleCheck(modInternal[i].entries);

            if (mod->lateInit) {
                kprintf("Performing the Late initialization for module %s\n",mod->name);

                LateInit_t init = (LateInit_t)mod->lateInit;
                init();
            }
            MmuUnmapPage(moduleAddr);
            LoadCr3(oldCr3);
        }
    }
}

