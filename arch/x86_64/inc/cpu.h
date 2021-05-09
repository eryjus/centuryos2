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
const uint64_t IF = (1<<9);


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
// -- Get a byte from an I/O Port
//    ---------------------------
inline uint8_t INB(uint16_t port) {
    uint8_t rv;
    __asm volatile ( "inb %1, %0" : "=a"(rv) : "Nd"(port) );
    return rv;
}


//
// -- function to write to an IO port
//    -------------------------------
inline void OUTB(uint16_t port, uint8_t val)
{
    __asm volatile ( "outb %0, %1" :: "a"(val), "Nd"(port) );
}


//
// -- Create a NOP instruction
//    ------------------------
inline void NOP(void)
{
    __asm volatile ("nop" ::: "memory");
}


//
// -- Halt the CPU
//    ------------
inline void HLT(void)
{
    __asm volatile ("hlt" ::: "memory");
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
// -- CPUID bits
//    ----------
const uint64_t CPUID_FEAT_ECX_SSE3         = (1<<0);
const uint64_t CPUID_FEAT_ECX_PCLMULQDQ    = (1<<1);
const uint64_t CPUID_FEAT_ECX_DTES64       = (1<<2);
const uint64_t CPUID_FEAT_ECX_MONITOR      = (1<<3);
const uint64_t CPUID_FEAT_ECX_DS_CPL       = (1<<4);
const uint64_t CPUID_FEAT_ECX_VMX          = (1<<5);
const uint64_t CPUID_FEAT_ECX_SMX          = (1<<6);
const uint64_t CPUID_FEAT_ECX_EIST         = (1<<7);
const uint64_t CPUID_FEAT_ECX_TM2          = (1<<8);
const uint64_t CPUID_FEAT_ECX_SSSE3        = (1<<9);
const uint64_t CPUID_FEAT_ECX_CNTX_ID      = (1<<10);
const uint64_t CPUID_FEAT_ECX_SDBG         = (1<<11);
const uint64_t CPUID_FEAT_ECX_FMA          = (1<<12);
const uint64_t CPUID_FEAT_ECX_CMPXCHG16B   = (1<<13);
const uint64_t CPUID_FEAT_ECX_XTPR_UPD_CTL = (1<<14);
const uint64_t CPUID_FEAT_ECX_PDCM         = (1<<15);
const uint64_t CPUID_FEAT_ECX_PCID         = (1<<17);
const uint64_t CPUID_FEAT_ECX_DCA          = (1<<18);
const uint64_t CPUID_FEAT_ECX_SSE4_1       = (1<<19);
const uint64_t CPUID_FEAT_ECX_SSE4_2       = (1<<20);
const uint64_t CPUID_FEAT_ECX_X2APIC       = (1<<21);
const uint64_t CPUID_FEAT_ECX_MOVBE        = (1<<22);
const uint64_t CPUID_FEAT_ECX_POPCNT       = (1<<23);
const uint64_t CPUID_FEAT_ECX_TSC_DEADLINE = (1<<24);
const uint64_t CPUID_FEAT_ECX_AESNI        = (1<<25);
const uint64_t CPUID_FEAT_ECX_XSAVE        = (1<<26);
const uint64_t CPUID_FEAT_ECX_OSXSAVE      = (1<<27);
const uint64_t CPUID_FEAT_ECX_AVX          = (1<<28);
const uint64_t CPUID_FEAT_ECX_F16C         = (1<<29);
const uint64_t CPUID_FEAT_ECX_RDRAND       = (1<<30);



const uint64_t CPUID_FEAT_EDX_FPU          = (1<<0);
const uint64_t CPUID_FEAT_EDX_VME          = (1<<1);
const uint64_t CPUID_FEAT_EDX_DE           = (1<<2);
const uint64_t CPUID_FEAT_EDX_PSE          = (1<<3);
const uint64_t CPUID_FEAT_EDX_TSC          = (1<<4);
const uint64_t CPUID_FEAT_EDX_MSR          = (1<<5);
const uint64_t CPUID_FEAT_EDX_PAE          = (1<<6);
const uint64_t CPUID_FEAT_EDX_MCE          = (1<<7);
const uint64_t CPUID_FEAT_EDX_CX8          = (1<<8);
const uint64_t CPUID_FEAT_EDX_APIC         = (1<<9);
const uint64_t CPUID_FEAT_EDX_SEP          = (1<<11);
const uint64_t CPUID_FEAT_EDX_MTRR         = (1<<12);
const uint64_t CPUID_FEAT_EDX_PGE          = (1<<13);
const uint64_t CPUID_FEAT_EDX_MCA          = (1<<14);
const uint64_t CPUID_FEAT_EDX_CMOV         = (1<<15);
const uint64_t CPUID_FEAT_EDX_PAT          = (1<<16);
const uint64_t CPUID_FEAT_EDX_PSE36        = (1<<17);
const uint64_t CPUID_FEAT_EDX_PSN          = (1<<18);
const uint64_t CPUID_FEAT_EDX_CLFSH        = (1<<19);
const uint64_t CPUID_FEAT_EDX_DS           = (1<<21);
const uint64_t CPUID_FEAT_EDX_ACPI         = (1<<22);
const uint64_t CPUID_FEAT_EDX_MMX          = (1<<23);
const uint64_t CPUID_FEAT_EDX_FXSR         = (1<<24);
const uint64_t CPUID_FEAT_EDX_SSE          = (1<<25);
const uint64_t CPUID_FEAT_EDX_SSE2         = (1<<26);
const uint64_t CPUID_FEAT_EDX_SS           = (1<<27);
const uint64_t CPUID_FEAT_EDX_HTT          = (1<<28);
const uint64_t CPUID_FEAT_EDX_TM           = (1<<29);
const uint64_t CPUID_FEAT_EDX_PBE          = (1<<31);



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


//
// -- here are the MSRs we are dealing with
//    -------------------------------------
const uint32_t IA32_APIC_BASE_MSR = 0x1b;
const uint32_t IA32_MTRR_DEF_TYPE = 0xfe;





//
// -- Some types for reading/writing memory-mapped I/O addresses
//    ----------------------------------------------------------
typedef volatile uint32_t VolatileUint32_t;
typedef volatile uint64_t VolatileUint64_t;


//
// -- Write to a Memory Mapped I/O Register
//    -------------------------------------
inline void POKE32(Addr_t regLocation, VolatileUint32_t data) { (*((VolatileUint32_t *)(regLocation)) = (data)); }


//
// -- Write to a 64-bit Memory Mapped I/O Register
//    --------------------------------------------
inline void POKE64(Addr_t regLocation, uint64_t data) { (*((volatile uint64_t *)(regLocation)) = (data)); }


//
// -- Read from a Memory Mapped I/O Register
//    --------------------------------------
inline volatile uint32_t PEEK32(Addr_t regLocation) { return (*((volatile uint32_t *)(regLocation))); }


//
// -- Read from a 64-bit Memory Mapped I/O Register
//    ---------------------------------------------
inline volatile uint64_t PEEK64(Addr_t regLocation) { return (*((volatile uint64_t *)(regLocation))); }



#endif

