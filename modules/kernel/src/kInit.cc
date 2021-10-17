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
#include "mmu.h"
#include "boot-interface.h"
#include "scheduler.h"
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
    extern BootInterface_t *loaderInterface;

    ProcessInitTable();
    SerialOpen();

    kprintf("Welcome!\n");

    IntInit();                          // init the interrupt table (hardware structure)
    VectorInit();                       // init the vector table (OS structure)
    InternalInit();                     // init the internal function table
    ServiceInit();                      // init the OS services table
    CpuInit();                          // init the cpus tables
    ProcessInit(loaderInterface);

//    InternalTableDump();
//    VectorTableDump();

    ModuleEarlyInit();
//    InternalTableDump();
//    VectorTableDump();
//    ServiceTableDump();

    EnableInt();
    ModuleLateInit();

    CurrentThread()->priority = (ProcPriority_t)PTY_IDLE;

    while (true) {
        __asm volatile ("hlt");
    }
}

