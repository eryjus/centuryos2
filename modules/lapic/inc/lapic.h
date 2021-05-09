//===================================================================================================================
//
//  lapic.h -- Local APIC functions and structions
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-May-05  Initial  v0.0.8   ADCL  Initial version
//
//===================================================================================================================


#pragma once

#include "types.h"
#include "boot-interface.h"


//
// -- Which kind of APIC are we talking about?
//    ----------------------------------------
typedef enum {
    XAPIC = 1,
    X2APIC = 2,
} ApicVersion_t;



//
// -- The registers that can be read/written in the Local APIC
//    --------------------------------------------------------
typedef enum {
    APIC_LOCAL_ID = 0x20,
    APIC_LOCAL_VERSION = 0x30,
    APIC_TPR = 0x80,
    APIC_APR = 0x90,
    APIC_PPR = 0xa0,
    APIC_EOI = 0xb0,
    APIC_RRD = 0xc0,
    APIC_LDR = 0xd0,
    APIC_DFR = 0xe0,
    APIC_SIVR = 0xf0,
    APIC_ISR_BASE = 0x100,
    APIC_TMR_BASE = 0x180,
    APIC_IRR_BASE = 0x200,
    APIC_ESR = 0x280,
    APIC_CMCI = 0x2f0,
    APIC_ICR1 = 0x300,
    APIC_ICR2 = 0x310,
    APIC_LVT_TIMER = 0x320,
    APIC_LVT_THERMAL_SENSOR = 0x330,
    APIC_LVT_PERF_COUNTING_REG = 0x340,
    APIC_LVT_LINT0 = 0x350,
    APIC_LVT_LINT1 = 0x360,
    APIC_LVT_ERROR = 0x370,
    APIC_TIMER_ICR = 0x380,
    APIC_TIMER_CCR = 0x390,
    APIC_TIMER_DCR = 0x3e0,
    APIC_SELF_IPI = 0x3f0,
} ApicRegister_t;



//
// -- The bit to enable the APIC is software
//    --------------------------------------
const uint32_t APIC_SOFTWARE_ENABLE = (1<<8);


//
// -- LVT Constants
//    -------------
const uint32_t APIC_LVT_MASKED = (1<<16);
const uint32_t APIC_LVT_TIMER_PERIODIC = (0b01 << 17);


//
// -- The APIC driver structure
//    -------------------------
typedef struct Apic_t {
    uint64_t baseAddr;
    uint32_t factor;

    int (*earlyInit)(BootInterface_t *loaderInterface);
    void (*init)(void);
    ApicVersion_t (*getVersion)(void);

    uint32_t (*readApicRegister)(ApicRegister_t);
    void (*writeApicRegister)(ApicRegister_t, uint32_t);

    bool (*checkIndexedStatus)(ApicRegister_t, uint8_t);
    void (*eoi)(void);
} Apic_t;


//
// -- This is the apic structure in use
//    ---------------------------------
extern Apic_t *apic;


//
// -- Include the dependent headers
//    -----------------------------
#if __has_include("x2apic.h")
#   include "x2apic.h"
#endif

#if __has_include("xapic.h")
#   include "xapic.h"
#endif

