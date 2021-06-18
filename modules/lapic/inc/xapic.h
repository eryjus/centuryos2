//===================================================================================================================
//
//  xapic.h -- Local xAPIC functions and structions
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-May-06  Initial  v0.0.8   ADCL  Initial version
//
//===================================================================================================================


#pragma once

#include "types.h"

extern Apic_t xapic;


//
// -- Local APIC MMIO Address
//    -----------------------
const Addr_t LAPIC_MMIO = 0xffffaffffffff000;


//
// -- Some constant values realted to the xAPIC and the x2APIC
//    --------------------------------------------------------
const uint64_t IA32_APIC_BASE_MSR__BSP = (1<<8);
const uint64_t IA32_APIC_BASE_MSR__EN = (1<<11);


#define __XAPIC__

