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
//  2021-Jan-19  Initial  v0.0.2   ADCL  Initial version
//
//===================================================================================================================


#ifndef USE_SERIAL
#define USE_SERIAL
#endif


#include "types.h"
#include "idt.h"
#include "serial.h"
#include "internal.h"
#include "printf.h"
#include "boot-interface.h"
#include "kernel-funcs.h"
#include "modules.h"


//
// -- local prototype
//    ---------------
extern "C" void kInit(void);


//
// -- Perform the kernel initialization
//    ---------------------------------
void kInit(void)
{
    SerialOpen();

    kprintf("Welcome!\n");

    IdtInstall();
    InternalInit();
    ServiceInit();            // similar to InternalInit();
    CpuInit();

    kprintf(".. Module Early Init:\n");
    ModuleEarlyInit();

    kprintf(".. enabling interrupts\n");
    EnableInt();

    ModuleLateInit();

    kprintf("Boot Complete!\n");
    while (true) {
        __asm volatile ("hlt");
    }
}

