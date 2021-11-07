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
    Addr_t krn_GetInternalHandler(int i);
    Return_t krn_SetInternalHandler(int i, Addr_t handler, Addr_t cr3, Addr_t stack);
    void InternalTableDump(void);

    // -- Vector Handlers
    Addr_t krn_GetVectorHandler(int i);
    Return_t krn_SetVectorHandler(int i, Addr_t handler, Addr_t cr3, Addr_t stack);
    void VectorTableDump(void);

    // -- OS Services
    void ServiceInit(void);
    Addr_t krn_GetServiceHandler(int i);
    Return_t krn_SetServiceHandler(int i, Addr_t service, Addr_t cr3, Addr_t stack);
    void ServiceTableDump(void);

    // -- additional functions
    void *krn_AllocAndCopy(void *mem, size_t size);
    Return_t krn_ReleaseCopy(void *mem);
}


#endif
