//===================================================================================================================
//
//  cpu.cc -- Complete the CPU structure initialization
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jun-18  Initial  v0.0.9c  ADCL  Initial version
//
//===================================================================================================================




#include "types.h"
#include "printf.h"
#include "boot-interface.h"
#include "mmu.h"
#include "kernel-funcs.h"
#include "scheduler.h"
#include "stacks.h"
#include "cpu.h"



//
// -- The cpus abstraction structure
//    ------------------------------
ArchCpu_t cpus[MAX_CPU] = { {0} };



//
// -- The CPU that is currently starting
//    ----------------------------------
int cpuStarting = 0;



//
// -- The number of active CPUs
//    -------------------------
volatile int cpusActive = 0;



//
// -- return the number of active cores
//    ---------------------------------
int krn_ActiveCores(void)
{
    return cpusActive;
}


//
// -- Initialize the CPU structures to initial values
//    -----------------------------------------------
void CpuInit(void)
{
    cpusActive = 1;

    for (int i = 0; i < MAX_CPU; i ++) {
        cpus[i].cpuNum = i;
        cpus[i].cpu = &cpus[i];
        cpus[i].process = 0;
    }
}



extern "C" void kInitAp(void);


//
// -- Start the Application Processors
//    --------------------------------
void CpuApStart(BootInterface_t *interface)
{
    extern uint8_t SMP_START[];

    cmn_MmuMapPage(TRAMP_OFF, TRAMP_OFF >> 12, PG_WRT);
    kMemMoveB((void *)TRAMP_OFF, (void *)SMP_START, PAGE_SIZE);

    // -- only the actual trampoline code remains mapped.
    typedef struct TrampLoader_t {
        uint8_t jumpCode[8];
        uint32_t apLock;
        uint32_t pml4;
        uint64_t kStack;
        uint64_t kEntry;
    } __attribute__((packed)) TrampLoader_t;
    TrampLoader_t *trampLoader = (struct TrampLoader_t *)TRAMP_OFF;

    trampLoader->apLock = 1;            // -- lock the CPUs until we are actually ready
    trampLoader->pml4 = interface->bootVirtAddrSpace;
    trampLoader->kEntry = (Addr_t)kInitAp;

    //
    // -- Now, the trampoline code has been located in the correct place and the
    //    required data elements have been updated.  With that, we are ready to try
    //    to spin up additional CPUs.

    cpus[0].location = LapicGetId();
    // -- may need to set the rsp0 here!

    for (int i = 1; i < interface->cpuCount; i ++) {
#if DEBUG_ENABLED(CpuApStart)

        kprintf("Preparing to start CPU %d\n", i);

#endif

        Addr_t kStack = KrnStackFind();
        trampLoader->kStack = kStack + STACK_SIZE;

#if DEBUG_ENABLED(CpuApStart)

        kprintf(".. Preparing a stack at %p\n", kStack);

#endif

        for (int j = 0; j < STACK_SIZE; j += PAGE_SIZE) {
            MmuMapPage(kStack + j, PmmAlloc(), PG_WRT | PG_KRN);
        }

        cpuStarting = i;
        AtomicSet(&cpus[i].state, CPU_STARTING);

#if DEBUG_ENABLED(CpuApStart)

        kprintf(".. Sending INIT\n");

#endif

        IpiSendInit(i);

#if DEBUG_ENABLED(CpuApStart)

        kprintf(".. Sending Startup INI\n");

#endif

        IpiSendSipi(i, TRAMP_OFF);

        uint64_t timeout = TmrCurrentCount() + 500000;
        cpus[i].lastTimer = timeout;

        // -- wait here until the CPU reports it has started.
        while (AtomicRead(&cpus[i].state) == CPU_STARTING) {
            if (TmrCurrentCount() > timeout) {
                AtomicSet(&cpus[i].state, CPU_STOPPED);
                continue;
            }
        }

        cpusActive ++;

#if DEBUG_ENABLED(CpuApStart)

        kprintf("CPU %d has reported its startup is complete\n", i);
        kprintf(".. There are now %d CPUs active\n", cpusActive);

#endif
    }
}

