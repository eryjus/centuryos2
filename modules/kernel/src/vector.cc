//===================================================================================================================
//
//  vector.cc -- handle interfacing with the different vector handlers
//
//  These vector handlers are abstracted, so there should not be any arch-specific code herein.
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Sep-10  Initial  v0.0.9d  ADCL  Initial version
//
//===================================================================================================================


#ifndef USE_SERIAL
#define USE_SERIAL
#endif

#include "types.h"
#include "printf.h"
#include "serial.h"
#include "internals.h"
#include "scheduler.h"
#include "kernel-funcs.h"
#include "idt.h"


//
// -- the interrupt vector table
//    --------------------------
ServiceRoutine_t vectorTable[256] = { { 0 }};



//
// -- Get an interrupt vector handler
//    -------------------------------
Addr_t krn_GetVectorHandler(int, int i)
{
    if (i < 0 || i >= 256) return -EINVAL;

    kprintf("Getting internal handler: %p\n", vectorTable[i].handler);

    return vectorTable[i].handler;
}




//
// -- Set an interrupt vector handler
//    -------------------------------
Return_t krn_SetVectorHandler(int, int i, Addr_t handler, Addr_t cr3, Addr_t stack)
{
    if (i < 0 || i >= 256) return -EINVAL;

    kprintf("Setting vector handler %d to %p from %p\n", i, handler, cr3);

    vectorTable[i].handler = handler;
    vectorTable[i].cr3 = cr3;
    vectorTable[i].stack = stack;
    vectorTable[i].runtimeRegs = 0;
    vectorTable[i].lock = {0};

    return 0;
}



//
// -- This is the timer vector
//    ------------------------
extern "C" void TimerVector(Addr_t *);
void TimerVector(Addr_t *)
{
    uint64_t now = TmrTick();
    TmrEoi();
    sch_Tick(0, now);
}



//
// -- The remaining functions need to have access to kprintf
//    ------------------------------------------------------
#ifdef kprintf
#   undef kprintf
    extern "C" int kprintf(const char *fmt, ...);
#endif



//
// -- Initialize the vector table
void VectorInit(void)
{
    krn_SetVectorHandler(0,  0, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0,  1, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0,  2, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0,  3, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0,  4, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0,  5, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0,  6, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0,  7, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0,  8, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0,  9, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 10, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 11, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 12, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 13, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 14, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 15, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 16, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 17, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 18, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 19, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 20, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 21, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 22, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 23, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 24, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 25, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 26, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 27, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 28, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 29, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 30, (Addr_t)IdtGenericHandler, 0, 0);
    krn_SetVectorHandler(0, 31, (Addr_t)IdtGenericHandler, 0, 0);

    krn_SetVectorHandler(0, 32, (Addr_t)TimerVector, 0, 0);
}



//
// -- Dump the contents of the Interrupt Service Table
//    ------------------------------------------------
void VectorTableDump(void)
{
    kprintf("IRQ Table Contents:\n");

    for (int i = 0; i < 256; i ++) {
        if (vectorTable[i].handler != 0 || vectorTable[i].cr3 != 0) {
            kprintf("  %d: %p from context %p (Stack: %p)\n", i, vectorTable[i].handler,
                    vectorTable[i].cr3, vectorTable[i].stack);
        }
    }
}



