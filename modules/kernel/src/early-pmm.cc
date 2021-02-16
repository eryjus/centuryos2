//====================================================================================================================
//
//  early-pmm.cc -- An early frame allocator before the PMM is put in charge
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-30  Initial  v0.0.4   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "boot-interface.h"
#include "printf.h"
#include "idt.h"


//
// -- Allocate an early frame from the pool
//    -------------------------------------
Frame_t PmmEarlyFrame(void)
{
    extern BootInterface_t *loaderInterface;

    kprintf(".. (new early frame)\n");

    return loaderInterface->nextEarlyFrame ++;
}

