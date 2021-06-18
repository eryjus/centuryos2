//===================================================================================================================
//
//  init-table.cc -- Process the initialization table function calls
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jun-18  Initial  v0.0.9c  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "kernel-funcs.h"



//
// -- Some internal types used to complete the initialization
//    -------------------------------------------------------
typedef void (*FunctionPtr_t)(void);
extern FunctionPtr_t const __init_array_start[], __init_array_end[];



//
// -- Process the initialization table
//    --------------------------------
void ProcessInitTable(void)
{
    FunctionPtr_t *wrk = (FunctionPtr_t *)__init_array_start;

    while (wrk != (FunctionPtr_t *)__init_array_end) {
        KernelPrintf("Calling init function at %p\n", *wrk);
        (*wrk)();                   // -- call the function
        wrk ++;
    }
}


