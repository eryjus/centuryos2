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
inline int GetInternalHandler(int number) { return InternalDispatch1(INT_GET_HANDLER, (Addr_t)number); }
inline int SetInternalHandler(int number, Addr_t handlerAddr) {
    return InternalDispatch2(INT_SET_HANDLER, (Addr_t)number, (Addr_t)handlerAddr);
}
inline Frame_t PmmAlloc(void) { return InternalDispatch0(INT_PMM_ALLOC); }



