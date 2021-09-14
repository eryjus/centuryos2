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
#include "internals.h"
#include "printf.h"
#include "boot-interface.h"
#include "kernel-funcs.h"
#include "modules.h"


//
// -- local prototype
//    ---------------
extern "C" void kInit(void);
extern BootInterface_t *loaderInterface;


//
// -- Perform the kernel initialization
//    ---------------------------------
void kInit(void)
{
    ProcessInitTable();
    SerialOpen();

    kprintf("Welcome!\n");

    IntInit();                          // init the interrupt table (hardware structure)
    VectorInit();                       // init the vector table (OS structure)
    InternalInit();                     // init the internal function table
    ServiceInit();                      // init the OS services table
    CpuInit();                          // init the cpus tables

    InternalTableDump();
    VectorTableDump();

    kprintf(".. Module Early Init:\n");
    ModuleEarlyInit();
    kprintf(".. loader Virtual Address Space (%p) vs. cr3 (%p)\n", loaderInterface->bootVirtAddrSpace, GetAddressSpace());

    InternalTableDump();
    VectorTableDump();
    ServiceTableDump();

    kprintf("Enabling interrupts\n");

    EnableInt();
    ModuleLateInit();

    kprintf("Boot Complete!\n");
    while (true) {
        __asm volatile ("hlt");
    }
}

