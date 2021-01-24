//===================================================================================================================
//
//  kInit.cc -- Complete the initialization
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-19  Initial  v0.0.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "idt.h"
#include "serial.h"
#include "internal.h"
#include "printf.h"


//
// -- local prototype
//    ---------------
extern "C" void kInit(void);


//
// -- Perform the kernel initialization
//    ---------------------------------
void kInit(void)
{
    SerialOpen();
    IdtInstall();
    InternalInit();

    kprintf("Welcome!\n");

    while (true) {}
}

