//===================================================================================================================
//
//  stacks.h -- Some helpers to managing kernel stacks
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  There are several kernel stack locations that need to be managed.  These will all use the same address space.
//  These functions will assist in this.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-May-17  Initial  v0.0.9b  ADCL  Initial version
//
//===================================================================================================================


#pragma once


#include "types.h"


extern "C" {
    //
    // -- Allocate a stack
    //    ----------------
    void StackAlloc(Addr_t stackBase);


    //
    // -- Release a stack
    //    ---------------
    void StackRelease(Addr_t stackBase);


    //
    // -- Find an available stack
    //    -----------------------
    Addr_t StackFind(void);
}



