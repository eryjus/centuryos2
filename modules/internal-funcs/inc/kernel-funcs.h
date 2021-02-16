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
//  2021-Jan-22  Initial  v0.0.3   ADCL  Initial version
//
//===================================================================================================================


#pragma once

#include "types.h"
#include "internal-bits.h"


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
// -- The actual interface function calls
//    -----------------------------------

// -- Function 0
inline int GetInternalHandler(int number) {
    return InternalDispatch1(INT_GET_HANDLER, (Addr_t)number);
}

// -- Function 1
inline int SetInternalHandler(int number, Addr_t handlerAddr, Addr_t cr3) {
    return InternalDispatch3(INT_SET_HANDLER, (Addr_t)number, (Addr_t)handlerAddr, cr3);
}

// -- Function 2
inline int GetInternalService(int number) {
    return InternalDispatch1(INT_GET_SERVICE, (Addr_t)number);
}

// -- Function 3
inline int SetInternalService(int number, Addr_t serviceAddr, Addr_t cr3) {
    return InternalDispatch3(INT_SET_SERVICE, (Addr_t)number, (Addr_t)serviceAddr, cr3);
}

// -- Function 6
inline int InternalMmuMap(Addr_t addr, Frame_t frame, bool writable) {
    return InternalDispatch3(INT_MMU_MAP, addr, (Addr_t)frame, (Addr_t)writable);
}

// -- Function 7
inline int InternalMmuUnmap(Addr_t addr) {
    return InternalDispatch1(INT_MMU_UNMAP, addr);
}

// -- Function 10
inline Frame_t PmmAllocAligned(bool lowMem, int numBitsAligned, size_t count) {
    return InternalDispatch3(INT_PMM_ALLOC, lowMem, numBitsAligned, count);
}
inline Frame_t PmmAllocLow(void) { return PmmAllocAligned(true, 12, 1); }
inline Frame_t PmmAlloc(void) { return PmmAllocAligned(false, 12, 1); }

// -- Function 11
inline int PmmReleaseRange(Frame_t frame, size_t count) {
    return InternalDispatch2(INT_PMM_RELEASE, frame, count);
}
inline int PmmRelease(Frame_t frame) { return PmmReleaseRange(frame, 1); }

