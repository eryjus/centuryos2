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
#include "internal-bits.h"



//
// -- define some function prototypes
//    -------------------------------
extern "C" {
    void InternalInit(void);
    int InternalGetHandler(int i);
    int InternalSetHandler(int i, InternalHandler_t handler, Addr_t cr3);
    void ServiceInit(void);
    int ServiceGetHandler(int i);
    int ServiceSetHandler(int i, ServiceHandler_t handler, Addr_t cr3);
    void InternalTableDump(void);
}



//
// -- The function table
//    ------------------
extern IdtHandlerFunc_t InternalTarget;


#endif
