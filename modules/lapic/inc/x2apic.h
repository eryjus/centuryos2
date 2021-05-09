//===================================================================================================================
//
//  x2apic.h -- Local x2APIC functions and structions
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

extern Apic_t x2apic;


//
// -- Constants specific to the x2APIC
//    --------------------------------
const uint64_t IA32_APIC_BASE_MSR__EXTD = (1<<10);

#define __X2APIC__

