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


//#define USE_SERIAL

#include "types.h"
#include "kernel-funcs.h"
#include "serial.h"
#include "mmu.h"


//
// -- Get a new Table
//    ---------------
Frame_t MmuGetTable(void)
{
    SerialPutString("Getting a frame...\n");
    Frame_t rv = PmmAlloc();

    SerialPutString("New table at frame ");
    SerialPutHex64(rv);
    SerialPutChar('\n');

    return rv;
}

