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
    Frame_t MmuGetTable(void);
    bool MmuIsMapped(Addr_t a);
    void krn_MmuUnmapPage(Addr_t a);
    void krn_MmuMapPage(Addr_t a, Frame_t f, bool writable);
}
