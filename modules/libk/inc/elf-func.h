//===================================================================================================================
//
//  elf-func.h -- Functions related to parsing an elf executable
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-04  Initial  v0.0.1   ADCL  Initial version
//
//===================================================================================================================


#pragma once

#include "types.h"


//
// -- function prototypes
//    -------------------
extern "C" {
    Addr_t ElfLoadImage(Addr_t location);
}

