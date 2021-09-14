//====================================================================================================================
//
//  internals.h -- handling internal interrupts (as in not user-facing)
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-20  Initial  v0.0.3   ADCL  Initial version
//
//===================================================================================================================


#pragma once
#ifndef __INTERNALS_H__
#define __INTERNALS_H__


#include "types.h"
#include "idt.h"



//
// -- the max number of internal functions
//    ------------------------------------
#define MAX_HANDLERS        1024


//
// -- This is a prototype for the internal function entry point
//    ---------------------------------------------------------
extern "C" int krn_KernelPrintf(const char *, ...);



//
// -- define some function prototypes
//    -------------------------------
extern "C" {
    // -- Internal Functions operations
    void InternalInit(void);
    Return_t krn_GetInternalHandler(int, int i);
    Return_t krn_SetInternalHandler(int, int i, Addr_t handler, Addr_t cr3, Addr_t stack);
    void InternalTableDump(void);

    // -- Vector Handlers
    Return_t krn_SetVectorHandler(int, int i, Addr_t handler, Addr_t cr3, Addr_t stack);
    Return_t krn_GetVectorHandler(int, int i);
    void VectorTableDump(void);

    // -- OS Services
    void ServiceInit(void);
    Return_t krn_GetServiceHandler(int, int i);
    Return_t krn_SetServiceHandler(int, int i, Addr_t service, Addr_t cr3, Addr_t stack);
    void ServiceTableDump(void);

    // -- additional functions
    void *krn_AllocAndCopy(int, void *mem, size_t size);
    Return_t krn_ReleaseCopy(int, void *mem);
}


#endif
