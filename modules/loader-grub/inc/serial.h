//===================================================================================================================
//
//  serial.h -- Functions related to Serial Output
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-13  Initial  v0.0.1   ADCL  Initial version
//
//===================================================================================================================


#pragma once

#include "types.h"


//
// -- function prototypes
//    -------------------
extern "C" {
    void SerialOpen(void);
    void SerialPutChar(uint8_t ch);
    void SerialPutString(const char *s);
    void SerialPutHex64(uint64_t h);
    void SerialPutHex32(uint32_t h);
}
