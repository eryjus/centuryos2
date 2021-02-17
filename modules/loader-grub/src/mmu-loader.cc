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
// -- function prototypes
//    -------------------
extern "C" {
    Frame_t MmuGetTable(void);
    bool MmuIsMapped(Addr_t a);
    void krn_MmuUnmapPage(Addr_t a);
    void krn_MmuMapPage(Addr_t a, Frame_t f, bool writable);

    void MmuMapPage(Addr_t a, Frame_t f, bool writable);
    void MmuUnmapPage(Addr_t a);
}


//
// -- allocate a new frame
//    --------------------
Frame_t MmuGetTable(void)
{
    extern Frame_t earlyFrame;
    return earlyFrame ++;
}


//
// -- Some wrapper functions
//    ----------------------
void MmuMapPage(Addr_t a, Frame_t f, bool writable) { krn_MmuMapPage(a, f, writable); }
void MmuUnmapPage(Addr_t a) { krn_MmuUnmapPage(a); }


