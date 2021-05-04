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
        pop %0\n        \
        cli\n           \
    " : "=m"(rv) :: "memory");

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
    if (i & IF) EnableInt();
}



//
// -- CPUID function -- lifted from: https://wiki.osdev.org/CPUID
//    issue a single request to CPUID. Fits 'intel features', for instance note that even if only "eax" and "edx"
//    are of interest, other registers will be modified by the operation, so we need to tell the compiler about it.
//    -------------------------------------------------------------------------------------------------------------
inline void CPUID(int code, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    __asm volatile("cpuid" : "=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d) : "a"(code) : "memory");
}


//
// -- Model Specific Registers
//    ------------------------
inline uint64_t RDMSR(uint32_t r) {
    uint64_t _lo, _hi;
    __asm volatile("rdmsr" : "=a"(_lo),"=d"(_hi) : "c"(r) : "%ebx", "memory");
    return (((uint64_t)_hi) << 32) | _lo;
}

inline void WRMSR(uint32_t r, uint64_t v) {
    uint32_t _lo = (uint32_t)(v & 0xffffffff);
    uint32_t _hi = (uint32_t)(v >> 32);
    __asm volatile("wrmsr" : : "c"(r),"a"(_lo),"d"(_hi) : "memory");
}

const uint32_t IA32_MTRR_DEF_TYPE = 0xfe;


#endif

