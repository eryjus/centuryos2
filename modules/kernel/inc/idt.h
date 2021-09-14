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
#ifndef __IDT_H__
#define __IDT_H__


#include "types.h"


//
// -- Function prototypes
//    -------------------
extern "C" {
    void IntInit(void);
    void VectorInit(void);
    Frame_t PmmEarlyFrame(void);
    void __attribute__((noreturn)) IdtGenericHandler(ServiceRoutine_t *handler);
}


#endif

