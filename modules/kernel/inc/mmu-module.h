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
    int krn_MmuMapPage(int, Addr_t a, Frame_t f, int flags);
    int krn_MmuUnmapPage(int, Addr_t a);
    Return_t krn_MmuIsMapped(int, Addr_t);
    int krn_MmuDump(int, Addr_t);
    int krn_MmuMapPageEx(int, Addr_t space, Addr_t a, Frame_t f, int flags);
    int krn_MmuUnmapEx(int, Addr_t space, Addr_t a);
}


