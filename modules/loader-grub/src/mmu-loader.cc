//===================================================================================================================
//
//  mmu-loader.cc -- Trivial memory management function
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-03  Initial  v0.0.4   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "mmu.h"



//
// -- allocate a new frame
//    --------------------
Frame_t MmuGetTable(void)
{
    extern Frame_t earlyFrame;
    return earlyFrame ++;
}

