/****************************************************************************************************************//**
*   @file               serial.cc
*   @brief              Common serial functions independent of arch/plat
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Nov-16
*   @since              v0.0.13
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*   Handle interacting with the serial port for the pc platform.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Nov-16 | Initial |  v0.0.13 | ADCL | Initial version; separated from `plat-serial.cc`
*
*///=================================================================================================================



#include "types.h"
#include "serial.h"



/****************************************************************************************************************//**
*   @var                digits
*
*   @brief              This is a string of hexidecimal characters which is used for outputting hex numbers
*
*   @note               Only lower case hex numbers are supported.
*///-----------------------------------------------------------------------------------------------------------------
static const char *digits = "0123456789abcdef";




/********************************************************************************************************************
*   Documented in `serial.h`
*///-----------------------------------------------------------------------------------------------------------------
void SerialPutString(const char *s)
{
    while (*s) {
        SerialPutChar(*s ++);
    }
}



/********************************************************************************************************************
*   Documented in `serial.h`
*///-----------------------------------------------------------------------------------------------------------------
void SerialPutHex64(uint64_t h)
{
    SerialPutChar('0');
    SerialPutChar('x');
    SerialPutChar(digits[(h>>60) & 0xf]);
    SerialPutChar(digits[(h>>56) & 0xf]);
    SerialPutChar(digits[(h>>52) & 0xf]);
    SerialPutChar(digits[(h>>48) & 0xf]);
    SerialPutChar('_');
    SerialPutChar(digits[(h>>44) & 0xf]);
    SerialPutChar(digits[(h>>40) & 0xf]);
    SerialPutChar(digits[(h>>36) & 0xf]);
    SerialPutChar(digits[(h>>32) & 0xf]);
    SerialPutChar('_');
    SerialPutChar(digits[(h>>28) & 0xf]);
    SerialPutChar(digits[(h>>24) & 0xf]);
    SerialPutChar(digits[(h>>20) & 0xf]);
    SerialPutChar(digits[(h>>16) & 0xf]);
    SerialPutChar('_');
    SerialPutChar(digits[(h>>12) & 0xf]);
    SerialPutChar(digits[(h>> 8) & 0xf]);
    SerialPutChar(digits[(h>> 4) & 0xf]);
    SerialPutChar(digits[(h>> 0) & 0xf]);
}



/********************************************************************************************************************
*   Documented in `serial.h`
*///-----------------------------------------------------------------------------------------------------------------
void SerialPutHex32(uint32_t h)
{
    SerialPutChar('0');
    SerialPutChar('x');
    SerialPutChar(digits[(h>>28) & 0xf]);
    SerialPutChar(digits[(h>>24) & 0xf]);
    SerialPutChar(digits[(h>>20) & 0xf]);
    SerialPutChar(digits[(h>>16) & 0xf]);
    SerialPutChar('_');
    SerialPutChar(digits[(h>>12) & 0xf]);
    SerialPutChar(digits[(h>> 8) & 0xf]);
    SerialPutChar(digits[(h>> 4) & 0xf]);
    SerialPutChar(digits[(h>> 0) & 0xf]);
}

