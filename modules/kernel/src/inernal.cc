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


//#define USE_SERIAL

#include "types.h"
#include "spinlock.h"
#include "printf.h"
#include "mmu.h"
#include "internal.h"


//
// -- the max number of internal functions
//    ------------------------------------
#define MAX_HANDLERS        1024
int maxHandlers = MAX_HANDLERS;


//
// -- The internal handler tables
//    ---------------------------
typedef struct InternalFunctions_t {
    InternalHandler_t handler;
    Addr_t cr3;
} InternalFunctions_t;

InternalFunctions_t internalTable[MAX_HANDLERS] = { { 0 } };


//
// -- The service handler tables
//    --------------------------
typedef struct ServiceFunctions_t {
    ServiceHandler_t handler;
    Addr_t cr3;
} ServiceFunctions_t;

ServiceFunctions_t serviceTable[MAX_HANDLERS] = { { 0 } };



//
// -- Read an internal function handler address from the table
//    --------------------------------------------------------
int InternalGetHandler(int i)
{
    if (i < 0 || i >= maxHandlers) return -EINVAL;
    return (int)internalTable[i].handler;
}


//
// -- Set an internal function handler address in the table
//    -----------------------------------------------------
int InternalSetHandler(int i, InternalHandler_t handler, Addr_t cr3)
{
    if (i < 0 || i >= maxHandlers) return -EINVAL;
    kprintf("Setting internal handler %d to %p from %p\n", i, handler, cr3);
    internalTable[i].handler = handler;
    internalTable[i].cr3 = cr3;
    return 0;
}


//
// -- Read an service function handler address from the table
//    -------------------------------------------------------
int InternalGetService(int i)
{
    if (i < 0 || i >= maxHandlers) return -EINVAL;
    return (int)serviceTable[i].handler;
}


//
// -- Set an service function handler address in the table
//    ----------------------------------------------------
int InternalSetService(int i, ServiceHandler_t service, Addr_t cr3)
{
    if (i < 0 || i >= maxHandlers) return -EINVAL;
    serviceTable[i].handler = service;
    serviceTable[i].cr3 = cr3;
    return 0;
}


//
// -- Initialize the internal handler table
//    -------------------------------------
void InternalInit(void)
{
    for (int i = 0; i < MAX_HANDLERS; i ++) {
        internalTable[i].handler = (InternalHandler_t)NULL;
        internalTable[i].cr3 = 0;
    }

    internalTable[INT_GET_HANDLER].handler = (InternalHandler_t)InternalGetHandler;
    internalTable[INT_SET_HANDLER].handler = (InternalHandler_t)InternalSetHandler;
    internalTable[INT_GET_SERVICE].handler = (InternalHandler_t)InternalGetService;
    internalTable[INT_SET_SERVICE].handler = (InternalHandler_t)InternalSetService;
    internalTable[INT_MMU_MAP].handler = (InternalHandler_t)MmuMapPage;
    internalTable[INT_MMU_UNMAP].handler = (InternalHandler_t)MmuUnmapPage;
    internalTable[INT_PMM_ALLOC].handler = (InternalHandler_t)PmmEarlyFrame;
}


//
// -- Initialize the service handler table
//    ------------------------------------
void ServiceInit(void)
{
    for (int i = 0; i < MAX_HANDLERS; i ++) serviceTable[i].handler = (ServiceHandler_t)NULL;
}


//
// -- Dump the contents of the Internal Functions Table
//    -------------------------------------------------
void InternalTableDump(void)
{
    kprintf("Internal Table Contents:\n");

    for (int i = 0; i < MAX_HANDLERS; i ++) {
        if (internalTable[i].handler != 0 || internalTable[i].cr3 != 0) {
            kprintf("  %d: %p from context %p\n", i, internalTable[i].handler, internalTable[i].cr3);
        }
    }
}

