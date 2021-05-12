//===================================================================================================================
//
//  AssertFailure.cc -- Handle outputting that an assertion failed
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-May-11  Initial  0.0.9a   ADCL  Initial version -- Copied from Century-OS
//
//===================================================================================================================


#include "types.h"
#include "kernel-funcs.h"


//
// -- Handle outputting that an assertion failed
//    ------------------------------------------
bool AssertFailure(const char *expr, const char *msg, const char *file, int line)
{
    KernelPrintf("\n!!! ASSERT FAILURE !!!\n%s(%d) %s %s\n\n", file, line, expr, (msg?msg:""));

    // -- always return false in case this is used in a conditional
    return false;
}

