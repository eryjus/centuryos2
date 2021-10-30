//===================================================================================================================
//
//  boot-interface.h -- Communication structures between the loader and kernel
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-01  Initial  v0.0.4   ADCL  Initial version
//
//===================================================================================================================


#pragma once


#include "types.h"


//
// -- set the limit to the number of stuff we can handle
//    --------------------------------------------------
#define MAX_MODS    25
#define MAX_MEM     10


//
// -- This structure passes information between the loader and the kernel
//    -------------------------------------------------------------------
typedef struct BootInterface_t {
    Frame_t nextEarlyFrame;
    Addr_t bootVirtAddrSpace;
    int cpuCount;
    int modCount;
    Addr_t modAddr[MAX_MODS];
    struct {
        uint64_t start;
        uint64_t end;
    } memBlocks[MAX_MEM];
    int localApic;
} BootInterface_t;



//
// -- Some additional prototypes
//    --------------------------
extern "C" {
    Addr_t GetPageTables(void);
}


