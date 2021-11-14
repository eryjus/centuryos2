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


extern "C" void CpuDebugInit(void);


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
    ModuleEarlyInit();
InternalTableDump();
    CpuApStart(loaderInterface);
    cpus[0].lastTimer = TmrCurrentCount();

    EnableInt();
#if IS_ENABLED(KERNEL_DEBUGGER)
    CpuDebugInit();
#endif
    AtomicSet(&scheduler.enabled, 1);
    ModuleLateInit();

    // -- take on the Butler role
    CurrentThread()->priority = (ProcPriority_t)PTY_LOW;
    ksprintf(CurrentThread()->command, "Butler");

    while (true) {
        sch_ProcessBlock(PROC_MSGW);
    }
}


//
// -- For APs, start the OS on these processors
//    -----------------------------------------
extern "C" void kInitAp(void)
{
    int me = LapicGetId();

    assert(AtomicRead(&cpus[me].state) == CPU_STARTING);

    SchedulerCreateKInitAp(me);

    TmrApInit(NULL);
    EnableInt();
    kprintf("Confirming that CPU %d has started\n", me);
    AtomicSet(&cpus[me].state, CPU_STARTED);

    ProcessEnd();
    assert(false);
    while (true) {}
}
