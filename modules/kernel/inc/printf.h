//===================================================================================================================
//
//  printf.h -- A printf()-like function to write output to the serial port
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#pragma once
#ifndef _PRINTF_H__
#define __PRINTF_H__


#include "types.h"



//
// -- This function operates like printf()
//    ------------------------------------
#ifdef USE_SERIAL
extern "C" {
    int kprintf(const char *fmt, ...);
}
#else
#define kprintf(...)
#endif


#endif

