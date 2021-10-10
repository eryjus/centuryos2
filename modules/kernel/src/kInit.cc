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


Process_t *A;
Process_t *B;



//
// -- Process B
void ProcB(void)
{
    while (true) {
        kprintf("B");
//        SchProcessMilliSleep(333);
    }
}


void ProcC(void)
{
    kprintf("This is process C and it will terminate immediately.\n");
//    ProcessEnd();
}


//
// -- Perform the kernel initialization
//    ---------------------------------
void kInit(void)
{
    extern BootInterface_t *loaderInterface;

    ProcessInitTable();
    SerialOpen();

    kprintf("Welcome!\n");
    kprintf("\n");
    kprintf("For the record:\n");
    kprintf(".. offset of Process_t.addrspace = %p\n", offsetof(Process_t, virtAddrSpace));

    IntInit();                          // init the interrupt table (hardware structure)
    VectorInit();                       // init the vector table (OS structure)
    InternalInit();                     // init the internal function table
    ServiceInit();                      // init the OS services table
    CpuInit();                          // init the cpus tables
    ProcessInit(loaderInterface);

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

    A = CurrentThread();
    B = SchProcessCreate("B", (Addr_t)ProcB, GetAddressSpace());
    SchProcessCreate("C", (Addr_t)ProcC, GetAddressSpace());

    while (true) {
        kprintf("A");
//        SchProcessMilliSleep(250);
    }

    ModuleLateInit();

    kprintf("Boot Complete!\n");
    while (true) {
        __asm volatile ("hlt");
    }
}

