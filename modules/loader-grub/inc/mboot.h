//===================================================================================================================
//
//  mboot.h -- Functions related to parsing the multiboot info
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

#include "types.h"


//
// -- function prototypes
//    -------------------
extern "C" {
    Addr_t MBootGetKernel(void);
}

