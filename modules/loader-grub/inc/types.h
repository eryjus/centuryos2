//===================================================================================================================
//
//  types.h -- Foundational types for CenturyOS
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

#include <cstdint>
#include <cstddef>


//
// -- The Multiboot signatures
//    ------------------------
#define MB1SIG                          0x2badb002
#define MB2SIG                          0x36d76289


//
// -- Foundational Types
//    ------------------
typedef uint64_t Frame_t;
typedef uint64_t Addr_t;


