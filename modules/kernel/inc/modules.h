//===================================================================================================================
//
//  modules.h -- Interface for loading modules
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-02  Initial  v0.0.4   ADCL  Initial version
//
//===================================================================================================================


#pragma once

#include "types.h"


//
// -- function prototypes
//    -------------------
extern "C" {
    void ModuleEarlyInit(void);
    void ModuleLateInit(void);
}

