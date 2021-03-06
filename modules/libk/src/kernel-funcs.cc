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
int GetInternalHandler(int number)
{
    return InternalDispatch1(INT_GET_HANDLER, (Addr_t)number);
}


//
// -- Function 1 -- Set Interna Function Handler
//    ------------------------------------------
int SetInternalHandler(int number, Addr_t handlerAddr, Addr_t cr3)
{
    return InternalDispatch3(INT_SET_HANDLER, (Addr_t)number, (Addr_t)handlerAddr, cr3);
}


//
// -- Function 2 -- Get OK Service Handler
//    ------------------------------------
int GetInternalService(int number)
{
    return InternalDispatch1(INT_GET_SERVICE, (Addr_t)number);
}


//
// -- Function 3 -- Set OK Service Handler
//    ------------------------------------
int SetInternalService(int number, Addr_t serviceAddr, Addr_t cr3)
{
    return InternalDispatch3(INT_SET_SERVICE, (Addr_t)number, (Addr_t)serviceAddr, cr3);
}


//
// -- Function 6 -- Map a page
//    ------------------------
int MmuMapPage(Addr_t addr, Frame_t frame, bool writable)
{
    return InternalDispatch3(INT_MMU_MAP, addr, (Addr_t)frame, (Addr_t)writable);
}


//
// -- Function 7 -- Unmap a page
//    --------------------------
int MmuUnmapPage(Addr_t addr)
{
    return InternalDispatch1(INT_MMU_UNMAP, addr);
}


//
// -- Function 10 -- Allocate a number of aligned frames
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
// -- Function 11 -- Release a number of frames
//    -----------------------------------------
int PmmReleaseRange(Frame_t frame, size_t count)
{
    return InternalDispatch2(INT_PMM_RELEASE, frame, count);
}


//
// -- Release a single frame
//    ----------------------
int PmmRelease(Frame_t frame)
{
    return PmmReleaseRange(frame, 1);
}


//
// -- Function 16 -- Lock a Spinlock
//    ------------------------------
int SpinLock(Spinlock_t *lock)
{
    return InternalDispatch1(INT_SPIN_LOCK, (Addr_t)lock);
}


//
// -- Function 17 -- Try to lock a Spinlock with timeout
//    --------------------------------------------------
int SpinTry(Spinlock_t *lock, size_t timeout)
{
    return InternalDispatch2(INT_SPIN_TRY, (Addr_t)lock, timeout);
}


//
// -- Function 18 -- Unlock a Spinlock
//    --------------------------------
int SpinUnlock(Spinlock_t *lock)
{
    return InternalDispatch1(INT_SPIN_UNLOCK, (Addr_t)lock);
}

