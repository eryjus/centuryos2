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
    int krn_MmuUnmapPage(Addr_t a);
    int krn_MmuMapPage(Addr_t a, Frame_t f, int flags);
    int krn_MmuDumpTables(Addr_t);
    bool krn_MmuIsMapped(Addr_t);
}
