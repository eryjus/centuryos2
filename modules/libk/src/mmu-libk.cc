//===================================================================================================================
//
//  mmu-libk.cc -- Functions related to managing the Paging Tables specific to the non-loader
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-03  Initial  v0.0.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "kernel-funcs.h"
#include "mmu.h"


//
// -- Get a new Table
//    ---------------
Frame_t MmuGetTable(void)
{
    return PmmAlloc();
}

