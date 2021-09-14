//===================================================================================================================
//
//  lInit.cc -- Complete the initialization
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
#include "elf.h"
#include "mboot.h"
#include "serial.h"
#include "mmu.h"
#include "boot-interface.h"


#define PG_NONE     (0)
#define PG_WRT      (1<<0)
#define PG_KRN      (1<<1)
#define PG_DEV      (1<<15)


//
// -- some local function prototype
//    -----------------------------
extern "C" {
    void lInit(void);
    void JumpKernel(Addr_t entry, Addr_t stack);
}


//
// -- Complete the initialization
//    ---------------------------
void lInit(void)
{
    extern BootInterface_t *kernelInterface;
    extern Frame_t earlyFrame;
    extern Addr_t pml4;
    const Addr_t interfaceLocation = 0xffff9ffffffff000;

    // -- create all the kernel PML4 entries
    SerialOpen();
    SerialPutString("Hello\n");

    for (int i = 0x100; i < 0x1ff; i ++) {
        if ((i >= 0x100 && i < 0x140) || (i >= 0x1f0 && i < 0x1ff)) {
            MmuEmptyPdpt(i);
        }
    }

    Frame_t fr = earlyFrame ++;
    MmuMapPage(interfaceLocation, fr, PG_WRT);
    kernelInterface = (BootInterface_t *)interfaceLocation;
    kernelInterface->modCount = 0;
    kernelInterface->bootVirtAddrSpace = pml4;
    kernelInterface->cpuCount = 1;

    for (int i = 0; i < MAX_MEM; i ++) {
        kernelInterface->memBlocks[i].start = kernelInterface->memBlocks[i].end = 0;
    }

    SerialPutString("Getting the kernel\n");
    SerialPutString("  cr3 = ");
    SerialPutHex64(GetCr3());
    SerialPutChar('\n');

    Addr_t kernel = MBootGetKernel();
    Addr_t stack = (earlyFrame);
    earlyFrame += 4;
    MmuMapPage(0xfffff80000000000, stack + 0, PG_WRT);
    MmuMapPage(0xfffff80000001000, stack + 1, PG_WRT);
    MmuMapPage(0xfffff80000002000, stack + 2, PG_WRT);
    MmuMapPage(0xfffff80000003000, stack + 3, PG_WRT);
    SerialPutString("Stack mapped\n");

    if (kernel != 0) {
        Addr_t entry = ElfLoadImage(kernel);

        kernelInterface->nextEarlyFrame = earlyFrame;

        SerialPutString("Jumping!\n");

        JumpKernel(entry, STACK_LOCATION + STACK_SIZE);
    }

    while (true) {}
}

