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
//  2021-Jan-03  Initial  v0.0.1   ADCL  Initial version
//
//===================================================================================================================


#pragma once



//
// -- function prototypes
//    -------------------
extern "C" {
    int krn_MmuMapPage(Addr_t a, Frame_t f, int flags);
    int krn_MmuUnmapPage(Addr_t a);
    Return_t krn_MmuIsMapped(Addr_t);
    int krn_MmuDump(Addr_t);
    int krn_MmuMapPageEx(Addr_t space, Addr_t a, Frame_t f, int flags);
    int krn_MmuUnmapEx(Addr_t space, Addr_t a);
}


