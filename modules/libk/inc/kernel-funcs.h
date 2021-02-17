//====================================================================================================================
//
//  kernel-funcs.h -- prototypes for the kernel internal functions
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-16  Initial  v0.0.6   ADCL  Initial version (relocated)
//
//===================================================================================================================


#pragma once

#include "types.h"



//
// -- these are the internal functions provided by the kernel
//    -------------------------------------------------------
enum {
    INT_GET_HANDLER     = 0,
    INT_SET_HANDLER     = 1,
    INT_GET_SERVICE     = 2,
    INT_SET_SERVICE     = 3,
    INT_GET_INTERRUPT   = 4,
    INT_SET_INTERRUPT   = 5,
    INT_MMU_MAP         = 6,
    INT_MMU_UNMAP       = 7,
    INT_PMM_ALLOC       = 10,
    INT_PMM_RELEASE     = 11,
    INT_SPIN_LOCK       = 16,
    INT_SPIN_TRY        = 17,
    INT_SPIN_UNLOCK     = 18,
};



//
// -- These are the interface functions
//    ---------------------------------
extern "C" {
    int InternalDispatch0(int func);
    int InternalDispatch1(int func, Addr_t p1);
    int InternalDispatch2(int func, Addr_t p1, Addr_t p2);
    int InternalDispatch3(int func, Addr_t p1, Addr_t p2, Addr_t p3);
    int InternalDispatch4(int func, Addr_t p1, Addr_t p2, Addr_t p3, Addr_t p4);
    int InternalDispatch5(int func, Addr_t p1, Addr_t p2, Addr_t p3, Addr_t p4, Addr_t p5);
}



//
// -- all the function prototypes
//    ---------------------------
extern "C" {
    // -- Function 0
    int GetInternalHandler(int number);

    // -- Function 1
    int SetInternalHandler(int number, Addr_t handlerAddr, Addr_t cr3);

    // -- Function 2
    int GetInternalService(int number);

    // -- Function 3
    int SetInternalService(int number, Addr_t serviceAddr, Addr_t cr3);

    // -- Function 6
    int MmuMapPage(Addr_t addr, Frame_t frame, bool writable);

    // -- Function 7
    int MmuUnmapPage(Addr_t addr);

    // -- Function 10
    Frame_t PmmAllocAligned(bool lowMem, int numBitsAligned, size_t count);
    Frame_t PmmAllocLow(void);
    Frame_t PmmAlloc(void);

    // -- Function 11
    int PmmReleaseRange(Frame_t frame, size_t count);
    int PmmRelease(Frame_t frame);

    // -- Function 16
    int SpinLock(Spinlock_t *lock);

    // -- Function 17
    int SpinTry(Spinlock_t *lock, size_t timeout);

    // -- Function 18
    int SpinUnlock(Spinlock_t *lock);
}

