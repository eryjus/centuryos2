//====================================================================================================================
//
//  kernel-funcs.cc -- Functions to interface with function calls
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-16  Initial  v0.0.6   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "kernel-funcs.h"


//
// -- The actual interface function calls
//    -----------------------------------


//
// -- Function 0 -- Get Internal Function handler
//    -------------------------------------------
Return_t GetInternalHandler(int number)
{
    return InternalDispatch1(INT_GET_INTERNAL, (Addr_t)number);
}


//
// -- Function 1 -- Set Internal Function Handler
//    -------------------------------------------
Return_t SetInternalHandler(int number, Addr_t handlerAddr, Addr_t cr3, Addr_t stack)
{
    return InternalDispatch4(INT_SET_INTERNAL, (Addr_t)number, (Addr_t)handlerAddr, cr3, stack);
}


//
// -- Function 2 -- Get Vector  handler address
//    -----------------------------------------
Return_t GetVectorHandler(int number)
{
    return InternalDispatch1(INT_GET_VECTOR, (Addr_t)number);
}


//
// -- Function 3 -- Set Vector handler address
//    ----------------------------------------
Return_t SetVectorHandler(int number, Addr_t interruptAddr, Addr_t cr3, Addr_t stack)
{
    return InternalDispatch4(INT_SET_VECTOR, (Addr_t)number, interruptAddr, cr3, stack);
}


//
// -- Function 4 -- Get OS Service Handler
//    ------------------------------------
Return_t GetServiceHandler(int number)
{
    return InternalDispatch1(INT_GET_SERVICE, (Addr_t)number);
}


//
// -- Function 5 -- Set OS Service Handler
//    ------------------------------------
Return_t SetServiceHandler(int number, Addr_t serviceAddr, Addr_t cr3, Addr_t stack)
{
    return InternalDispatch4(INT_SET_SERVICE, (Addr_t)number, (Addr_t)serviceAddr, cr3, stack);
}


//
// -- Function 16 -- Lock a Spinlock
//    ------------------------------
Return_t SpinLock(Spinlock_t *lock)
{
    return InternalDispatch1(INT_KRN_SPIN_LOCK, (Addr_t)lock);
}


//
// -- Function 17 -- Try to lock a Spinlock with timeout
//    --------------------------------------------------
Return_t SpinTry(Spinlock_t *lock, size_t timeout)
{
    return InternalDispatch2(INT_KRN_SPIN_TRY, (Addr_t)lock, timeout);
}


//
// -- Function 18 -- Unlock a Spinlock
//    --------------------------------
Return_t SpinUnlock(Spinlock_t *lock)
{
    return InternalDispatch1(INT_KRN_SPIN_UNLOCK, (Addr_t)lock);
}



//
// -- Function 24 -- Map a page
//    -------------------------
Return_t MmuMapPage(Addr_t addr, Frame_t frame, int flags)
{
    return InternalDispatch3(INT_KRN_MMU_MAP, addr, (Addr_t)frame, (Addr_t)flags);
}


//
// -- Function 25 -- Unmap a page
//    ---------------------------
Return_t MmuUnmapPage(Addr_t addr)
{
    return InternalDispatch1(INT_KRN_MMU_UNMAP, addr);
}


//
// -- Function 26 -- Is page mapped and if so to what Frame
//    -----------------------------------------------------
bool MmuIsMapped(Addr_t addr)
{
    return InternalDispatch1(INT_KRN_MMU_IS_MAPPED, addr);
}


//
// -- Function 27 -- Dump MMU Tables
//    ------------------------------
Return_t MmuDump(Addr_t addr)
{
    return InternalDispatch1(INT_KRN_MMU_DUMP, addr);
}



//
// -- Function 32 -- Copy memory into kernel space
//    --------------------------------------------
void *KrnCopyMem(void *mem, size_t size)
{
    return (void *)InternalDispatch2(INT_KRN_COPY_MEM, (Addr_t)mem, size);
}


//
// -- Function 33 -- Release memory back to kernel
//    --------------------------------------------
Return_t KrnReleaseMem(void *mem)
{
    return InternalDispatch1(INT_KRN_RLS_MEM, (Addr_t)mem);
}



//
// -- Function 64 -- Get Current Timer
//    --------------------------------
uint64_t TmrCurrentCount(void)
{
    return InternalDispatch0(INT_TMR_CURRENT_COUNT);
}



//
// -- Function 80 -- Allocate a number of aligned frames
//    --------------------------------------------------
Frame_t PmmAllocAligned(bool lowMem, int numBitsAligned, size_t count)
{
    return InternalDispatch3(INT_PMM_ALLOC, lowMem, numBitsAligned, count);
}

//
// -- Allocate a frames from low memory
//    ---------------------------------
Frame_t PmmAllocLow(void)
{
    return PmmAllocAligned(true, 12, 1);
}

//
// -- Allocate a frame
//    ----------------
Frame_t PmmAlloc(void)
{
    return PmmAllocAligned(false, 12, 1);
}



//
// -- Function 81 -- Release a number of frames
//    -----------------------------------------
Return_t PmmReleaseRange(Frame_t frame, size_t count)
{
    return InternalDispatch2(INT_PMM_RELEASE, frame, count);
}

//
// -- Release a single frame
//    ----------------------
Return_t PmmRelease(Frame_t frame)
{
    return PmmReleaseRange(frame, 1);
}











//
// -- Function 25 -- Scheduler Timer Tick
//    -----------------------------------
Return_t SchTimerTick(uint64_t now)
{
    return InternalDispatch1(INT_SCH_TICK, now);
}


//
// -- Function 26 -- Block a process
//    ------------------------------
Return_t SchProcessBlock(int status)
{
    return InternalDispatch1(INT_SCH_BLOCK, status);
}


//
// -- Function 27 -- Ready a process
//    ------------------------------
Return_t SchProcessReady(Process_t *proc)
{
    return InternalDispatch1(INT_SCH_READY, (Addr_t)proc);
}


//
// -- Function 28 -- Unblock a process
//    --------------------------------
Return_t SchProcessUnblock(Process_t *proc)
{
    return InternalDispatch1(INT_SCH_UNBLOCK, (Addr_t)proc);
}


//
// -- Function 29 -- Sleep a process for some microseconds
//    ----------------------------------------------------
Return_t SchProcessMicroSleepUntil(uint64_t when)
{
    return InternalDispatch1(INT_SCH_SLEEP_UNTIL, when);
}


//
// -- Function 30 -- Create a new process
//    -----------------------------------
Return_t SchProcessCreate(const char *name, void (*startingAddr)(void), Addr_t addrSpace)
{
    void *n = KrnCopyMem((void *)name, kStrLen(name) + 1);
    Return_t rv = InternalDispatch3(INT_SCH_CREATE, (Addr_t)n, (Addr_t)startingAddr, addrSpace);
    KrnReleaseMem(n);
    return rv;
}


