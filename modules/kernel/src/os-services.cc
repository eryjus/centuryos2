//====================================================================================================================
//
//  os-services.cc -- handling OS services as function calls from User Space
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Sep-03  Initial  v0.0.9d  ADCL  Initial version -- copied out of `internal.cc`
//
//===================================================================================================================



#include "types.h"
#include "spinlock.h"
#include "printf.h"
#include "mmu.h"
#include "scheduler.h"
#include "kernel-funcs.h"
#include "internals.h"



//
// -- This is the OS Services handler table
//    -------------------------------------
ServiceRoutine_t serviceTable[MAX_HANDLERS] = { { 0 } };



//
// -- Read an service function handler address from the table
//    -------------------------------------------------------
Addr_t krn_GetServiceHandler(int, int i)
{
    if (i < 0 || i >= MAX_HANDLERS) return -EINVAL;

    kprintf("Getting service handler: %p\n", serviceTable[i].handler);

    return serviceTable[i].handler;
}



//
// -- Set an service function handler address in the table
//    ----------------------------------------------------
Return_t krn_SetServiceHandler(int, int i, Addr_t service, Addr_t cr3, Addr_t stack)
{
    if (i < 0 || i >= MAX_HANDLERS) return -EINVAL;

    kprintf("Setting service handler %d to %p from %p\n", i, service, cr3);

    serviceTable[i].handler = service;
    serviceTable[i].cr3 = cr3;
    serviceTable[i].stack = stack;
    serviceTable[i].runtimeRegs = 0;
    serviceTable[i].lock = {0};

    return 0;
}



//
// -- Initialize the service handler table
//    ------------------------------------
void ServiceInit(void)
{
    for (int i = 0; i < MAX_HANDLERS; i ++) serviceTable[i].handler = (Addr_t)NULL;
}


//
// -- The remaining functions need to have access to kprintf() at all times
//    ---------------------------------------------------------------------
#ifdef kprintf
#   undef kprintf
    extern "C" int kprintf(const char *fmt, ...);
#endif


//
// -- Dump the contents of the OS Service Table
//    -----------------------------------------
void ServiceTableDump(void)
{
    kprintf("Service Table Contents:\n");

    for (int i = 0; i < MAX_HANDLERS; i ++) {
        if (serviceTable[i].handler != 0 || serviceTable[i].cr3 != 0) {
            kprintf("  %d: %p from context %p on stack %p\n", i, serviceTable[i].handler,
                    serviceTable[i].cr3, serviceTable[i].stack);
        }
    }
}



