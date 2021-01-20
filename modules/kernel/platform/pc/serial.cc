//===================================================================================================================
//
//  serial.cc -- serial functions for the x86_64-pc
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-13  Initial  v0.0.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial.h"

#ifdef USE_SERIAL

//
// -- Some local variables
//    --------------------
static uint16_t base = 0x3f8;
static const char *digits = "0123456789abcdef";



//
// -- Get a byte from an I/O Port
//    ---------------------------
inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}


//
// -- function to write to an IO port
//    -------------------------------
inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}


//
// -- Open the serial port
//    --------------------
void SerialOpen(void)
{
    outb(base + 1, 0x00);       // Disable all interrupts
    outb(base + 3, 0x80);       // Enable DLAB (set baud rate divisor)
    outb(base + 0, 0x01);       // Set divisor to 1 (lo byte) 115200 baud
    outb(base + 1, 0x00);       //                  (hi byte)
    outb(base + 3, 0x03);       // 8 bits, no parity, one stop bit
    outb(base + 2, 0xC7);       // Enable FIFO, clear them, with 14-byte threshold
    outb(base + 4, 0x0B);       // IRQs enabled, RTS/DSR set
}



//
// -- Output a single character to the serial port
//    --------------------------------------------
void SerialPutChar(uint8_t ch)
{
    if (ch == '\n') SerialPutChar('\r');

    while ((inb(base + 5) & 0x20) == 0) {}

    outb(base + 0, ch);
}


//
// -- Output a string of characters to the serial port
//    ------------------------------------------------
void SerialPutString(const char *s)
{
    while (*s) {
        SerialPutChar(*s ++);
    }
}


//
// -- Output a 64-bit hex number to the serial port
//    ---------------------------------------------
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


//
// -- Output a 32-bit hex number to the serial port
//    ---------------------------------------------
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

#endif
