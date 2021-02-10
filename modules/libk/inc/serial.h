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
#ifndef __SERIAL_H__
#define __SERIAL_H__


#include "types.h"


extern "C" {
    void SerialOpen(void);
}


#ifdef USE_SERIAL


//
// -- function prototypes
//    -------------------
extern "C" {
    void SerialPutChar(uint8_t ch);
    void SerialPutString(const char *s);
    void SerialPutHex64(uint64_t h);
    void SerialPutHex32(uint32_t h);
}


#else

#define SerialPutChar(ch) ((void)(ch))
#define SerialPutString(s) ((void)(s))
#define SerialPutHex64(h) ((void)(h))
#define SerialPutHex32(h) ((void)(h))

#endif

#endif
