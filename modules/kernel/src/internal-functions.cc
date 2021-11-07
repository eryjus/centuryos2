//====================================================================================================================
//
//  internal-functions.cc -- handling internal functions between services (as in not user-facing)
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Sep-03  Initial  v0.0.9d  ADCL  Initial version -- copied out of `internal.cc`
//
//===================================================================================================================



#include "types.h"
#include "spinlock.h"
#include "printf.h"
#include "mmu.h"
#include "scheduler.h"
#include "heap.h"
#include "kernel-funcs.h"
#include "stacks.h"
#include "internals.h"



//
// -- The internal handler table
//    --------------------------
ServiceRoutine_t internalTable[MAX_HANDLERS] = { { 0 } };



//
// -- Read an internal function handler address from the table
//    --------------------------------------------------------
Addr_t krn_GetInternalHandler(int i)
{
    if (i < 0 || i >= MAX_HANDLERS) return -EINVAL;

    kprintf("Getting internal handler: %p\n", internalTable[i].handler);

    return internalTable[i].handler;
}



//
// -- Set an internal function handler address in the table
//    -----------------------------------------------------
Return_t krn_SetInternalHandler(int i, Addr_t handler, Addr_t cr3, Addr_t stack)
{
    if (i < 0 || i >= MAX_HANDLERS) return -EINVAL;

    kprintf("Setting internal handler %d to %p from %p\n", i, handler, cr3);

    internalTable[i].handler = handler;
    internalTable[i].cr3 = cr3;
    internalTable[i].stack = stack;
    internalTable[i].runtimeRegs = 0;

    return 0;
}


//
// -- Allocate some space from the kernel heap and fill it
//    ----------------------------------------------------
void *krn_AllocAndCopy(void *mem, size_t size)
{
    if (!krn_MmuIsMapped((Addr_t)mem)) return 0;
    void *dest = HeapAlloc(size, false);
    kMemMoveB(dest, mem, size);
    return dest;
}


//
// -- Release allocated memory back to the kernel heap
//    ------------------------------------------------
Return_t krn_ReleaseCopy(void *mem)
{
    HeapFree(mem);
    return 0;
}


//
// -- Assume the debugger is not installed until overridden
//    -----------------------------------------------------
Return_t krn_DebuggerInstalled(void)
{
    return false;
}



//
// -- Signal the other cores to stop and wait for confirmation that they have
//    -----------------------------------------------------------------------
extern "C" Addr_t krn_PauseCores(void)
{
    extern AtomicInt_t coresEngaged;

    Addr_t flags = DisableInt();
    AtomicSet(&coresEngaged, 1);

    IpiSendIpi(IPI_PAUSE_CORES);
    int active = KrnActiveCores();
    while (AtomicRead(&coresEngaged) != active) {}

    return flags;
}



//
// -- Release the other cores from a stopped state
//    --------------------------------------------
extern "C" Return_t krn_ReleaseCores(Addr_t flags)
{
    extern AtomicInt_t coresEngaged;

    AtomicSet(&coresEngaged, 0);
    RestoreInt(flags);

    return 0;
}



//
// -- The remaining functions need to have access to kprintf() at all times
//    ---------------------------------------------------------------------
#ifdef kprintf
#   undef kprintf
    extern "C" int kprintf(const char *fmt, ...);
#endif


//
// -- Initialize the internal handler table
//    -------------------------------------
void InternalInit(void)
{

    for (int i = 0; i < MAX_HANDLERS; i ++) {
        internalTable[i].handler = (Addr_t)NULL;
        internalTable[i].cr3 = 0;
        internalTable[i].stack = 0;
        internalTable[i].runtimeRegs = 0;
        internalTable[i].lock = {0};
    }

    internalTable[INT_GET_INTERNAL].handler =       (Addr_t)krn_GetInternalHandler;
    internalTable[INT_SET_INTERNAL].handler =       (Addr_t)krn_SetInternalHandler;
    internalTable[INT_GET_VECTOR].handler =         (Addr_t)krn_GetVectorHandler;
    internalTable[INT_SET_VECTOR].handler =         (Addr_t)krn_SetVectorHandler;
    internalTable[INT_GET_SERVICE].handler =        (Addr_t)krn_GetServiceHandler;
    internalTable[INT_SET_SERVICE].handler =        (Addr_t)krn_SetServiceHandler;

    internalTable[INT_PRINTF].handler =             (Addr_t)krn_KernelPrintf;

    internalTable[INT_KRN_SPIN_LOCK].handler =      (Addr_t)krn_SpinLock;
    internalTable[INT_KRN_SPIN_TRY].handler =       (Addr_t)krn_SpinTry;
    internalTable[INT_KRN_SPIN_UNLOCK].handler =    (Addr_t)krn_SpinUnlock;

    internalTable[INT_KRN_MMU_MAP].handler =        (Addr_t)krn_MmuMapPage;
    internalTable[INT_KRN_MMU_UNMAP].handler =      (Addr_t)krn_MmuUnmapPage;
    internalTable[INT_KRN_MMU_IS_MAPPED].handler =  (Addr_t)krn_MmuIsMapped;
    internalTable[INT_KRN_MMU_DUMP].handler =       (Addr_t)krn_MmuDump;
    internalTable[INT_KRN_MMU_MAP_EX].handler =     (Addr_t)krn_MmuMapPageEx;
    internalTable[INT_KRN_MMU_MAP_EX].stack =       0xffffff0000008000 + 0x1000;
    internalTable[INT_KRN_MMU_MAP_EX].cr3 =         GetAddressSpace();
    internalTable[INT_KRN_MMU_UNMAP_EX].handler =   (Addr_t)krn_MmuUnmapEx;
    internalTable[INT_KRN_MMU_UNMAP_EX].stack =     0xffffff0000009000 + 0x1000;
    internalTable[INT_KRN_MMU_UNMAP_EX].cr3 =       GetAddressSpace();

    internalTable[INT_KRN_COPY_MEM].handler =       (Addr_t)krn_AllocAndCopy;
    internalTable[INT_KRN_RLS_MEM].handler =        (Addr_t)krn_ReleaseCopy;
    internalTable[INT_KRN_CORES_ACTIVE].handler =   (Addr_t)krn_ActiveCores;
    internalTable[INT_KRN_PAUSE_CORES].handler =    (Addr_t)krn_PauseCores;
    internalTable[INT_KRN_RELEASE_CORES].handler =  (Addr_t)krn_ReleaseCores;

    internalTable[INT_PMM_ALLOC].handler =          (Addr_t)PmmEarlyFrame;

    internalTable[INT_SCH_TICK].handler =           (Addr_t)sch_Tick;
    internalTable[INT_SCH_CREATE].handler =         (Addr_t)sch_ProcessCreate;
    internalTable[INT_SCH_READY].handler =          (Addr_t)sch_ProcessReady;
    internalTable[INT_SCH_BLOCK].handler =          (Addr_t)sch_ProcessBlock;
    internalTable[INT_SCH_UNBLOCK].handler =        (Addr_t)sch_ProcessUnblock;
    internalTable[INT_SCH_SLEEP_UNTIL].handler =    (Addr_t)sch_ProcessMicroSleepUntil;

    internalTable[INT_DBG_INSTALLED].handler =      (Addr_t)krn_DebuggerInstalled;


    krn_MmuMapPage(0xffffff0000008000, PmmAlloc(), PG_WRT);
    krn_MmuMapPage(0xffffff0000009000, PmmAlloc(), PG_WRT);
}



//
// -- Dump the contents of the Internal Functions Table
//    -------------------------------------------------
void InternalTableDump(void)
{
    kprintf("Internal Table Contents:\n");
    krn_MmuDump((Addr_t)internalTable);

    for (int i = 0; i < MAX_HANDLERS; i ++) {
        if (internalTable[i].handler != 0 || internalTable[i].cr3 != 0) {
            kprintf("  %d: %p from context %p on stack %p\n", i, internalTable[i].handler,
                    internalTable[i].cr3, internalTable[i].stack);
        }
    }
}

