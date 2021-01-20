//====================================================================================================================
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
//  2021-Jan-14  Initial  v0.0.2   ADCL  Initial version
//
//===================================================================================================================


#pragma once

#include <cstdint>
#include <cstddef>


#define USE_SERIAL


//
// -- Foundational Types
//    ------------------
typedef uint64_t Frame_t;
typedef uint64_t Addr_t;


//
// -- This is the type definition of a handler function
//    -------------------------------------------------
typedef void (*IdtHandlerFunc_t)(Addr_t *);



