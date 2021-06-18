//====================================================================================================================
//
//  internal.h -- handling internal interrupts (as in not user-facing)
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
#ifndef __INTERNAL_H__
#define __INTERNAL_H__


#include "types.h"
#include "idt.h"



//
// -- define some function prototypes
//    -------------------------------
extern "C" {
    void InternalInit(void);
    int krn_GetFunctionHandler(int i);
    int krn_SetFunctionHandler(int i, InternalHandler_t handler, Addr_t cr3);
    void ServiceInit(void);
    int ServiceGetHandler(int i);
    int ServiceSetHandler(int i, ServiceHandler_t handler, Addr_t cr3);
    void InternalTableDump(void);
    int krn_SetVectorHandler(int i, IdtHandlerFunc_t handler, Addr_t cr3);
}



//
// -- The function table
//    ------------------
extern IdtHandlerFunc_t InternalTarget;


//
// -- the interrupt vector table
//    --------------------------
typedef struct VectorFunctions_t {
    IdtHandlerFunc_t handler;
    Addr_t cr3;
} VectorFunctions_t;

extern VectorFunctions_t vectorTable[256];

#endif
