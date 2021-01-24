//====================================================================================================================
//
//  internal-bits.h -- this is the interface for internal functions
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-22  Initial  v0.0.3   ADCL  Initial version
//
//===================================================================================================================


#pragma once



//
// -- these are the internal functions provided by the kernel
//    -------------------------------------------------------
enum {
    INT_GET_HANDLER,
    INT_SET_HANDLER,
    INT_SPIN_LOCK,
    INT_SPIN_UNLOCK,
    INT_SPIN_TEST,
};





