//====================================================================================================================
//
//  cpu.h -- some tasks that are required at the cpu level
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


#pragma once
#ifndef __CPU_H__
#define __CPU_H__


#include "types.h"


//
// -- RFLAGS positions
//    ----------------
#define IF      (1<<9)


//
// -- disable interrupts, saving the current setting (priviledged)
//    ------------------------------------------------------------
inline Addr_t DisableInt(void) {
    Addr_t rv;
    __asm__ volatile (" \
        pushfq\n        \
        pop %%rax\n     \
        mov %0,%%rax\n  \
        cli\n           \
    " : "=m"(rv) : : "rax");

    rv &= IF;
    return rv;
}


//
// -- unilaterally enable interrupts (priviledged)
//    --------------------------------------------
inline void EnableInt(void) {
    __asm__ volatile ("sti");
}


//
// -- restore interrupts, enabling them only if there were previously enabled (priviledged)
//    -------------------------------------------------------------------------------------
inline void RestoreInt(Addr_t i) {
    if (i) EnableInt();
}


#endif

