//====================================================================================================================
//
//  internal.cc -- handling internal interrupts (as in not user-facing)
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-20  Initial  v0.0.3   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "internal.h"


//
// -- the max number of internal functions
//    ------------------------------------
#define MAX_HANDLERS        1024
int maxHandlers = MAX_HANDLERS;


//
// -- The internal handler table
//    --------------------------
InternalHandler_t internalTable[MAX_HANDLERS] = {0};



//
// -- Read an internal function handler address from the table
//    --------------------------------------------------------
int InternalGetHandler(int i)
{
    if (i < 0 || i >= maxHandlers) return -EINVAL;
    return (int)internalTable[i];
}


//
// -- Set an internal function handler address in the table
//    -----------------------------------------------------
int InternalSetHandler(int i, InternalHandler_t handler)
{
    if (i < 0 || i >= maxHandlers) return -EINVAL;
    internalTable[i] = handler;
    return 0;
}


//
// -- Initialize the internal handler table
//    -------------------------------------
void InternalInit(void)
{
    for (int i = 0; i < MAX_HANDLERS; i ++) internalTable[i] = (InternalHandler_t)NULL;

    internalTable[INT_GET_HANDLER] = (InternalHandler_t)InternalGetHandler;
    internalTable[INT_SET_HANDLER] = (InternalHandler_t)InternalSetHandler;
}


