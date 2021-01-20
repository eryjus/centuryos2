//====================================================================================================================
//
//  idt.h -- Interface points with the IDT
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-19  Initial  v0.0.2   ADCL  Initial version
//
//===================================================================================================================


#pragma once


#include "types.h"


//
// -- Function prototypes
//    -------------------
extern "C" {
    void IdtInstall(void);
    Addr_t IdtGetHandler(int i);
    void IdtSetHandler(int i, uint16_t sec, IdtHandlerFunc_t *handler, int ist, int dpl);
}
