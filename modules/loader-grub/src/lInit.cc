//===================================================================================================================
//
//  kInit.cc -- Complete the initialization
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


#include "types.h"
#include "elf.h"
#include "mboot.h"
#include "serial.h"
#include "mmu.h"
#include "boot-interface.h"


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

    SerialOpen();
    SerialPutString("Hello\n");

    Frame_t fr = earlyFrame ++;
    MmuMapPage(fr << 12, fr, true);
    kernelInterface = (BootInterface_t *)(fr << 12);
    kernelInterface->modCount = 0;

    Addr_t kernel = MBootGetKernel();

    SerialPutString("Kernel image at ");
    SerialPutHex32(kernel);
    SerialPutChar('\n');

    Addr_t stack = (earlyFrame);
    earlyFrame += 4;
    MmuMapPage(0xfffff80000000000, stack, true);
    MmuMapPage(0xfffff80000001000, stack + 1, true);
    MmuMapPage(0xfffff80000002000, stack + 2, true);
    MmuMapPage(0xfffff80000003000, stack + 3, true);

    SerialPutString("The new stack will be at frame ");
    SerialPutHex64((uint64_t)stack);
    SerialPutChar('\n');

    if (kernel != 0) {
        Addr_t entry = ElfLoadImage(kernel);

        kernelInterface->nextEarlyFrame = earlyFrame;

        SerialPutString("Jumping!\n");

        JumpKernel(entry, 0xfffff80000000000 + 0x4000);
    }

    while (true) {}
}

