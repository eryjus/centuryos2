//===================================================================================================================
//
//  mmu.h -- Functions related to managing the Paging Tables
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-16  Initial  v0.0.6   ADCL  Initial version
//
//===================================================================================================================


#pragma once

#include "types.h"


//
// -- function prototypes
//    -------------------
extern "C" {
    Frame_t MmuGetTable(void);
    bool MmuIsMapped(Addr_t a);
    void MmuUnmapPage(Addr_t a);
    void MmuMapPage(Addr_t a, Frame_t f, int flags);
    void ldr_MmuUnmapPage(Addr_t a);
    void ldr_MmuMapPage(Addr_t a, Frame_t f, int flags);
    void MmuEmptyPdpt(int index);
}
