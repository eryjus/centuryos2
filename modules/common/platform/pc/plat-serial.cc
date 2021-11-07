/****************************************************************************************************************//**
*   @file               plat-serial.cc
*   @brief              Serial functions for the x86_64-pc
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Jan-13
*   @since              v0.0.01
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
*   | 2021-Jan-13 | Initial |  v0.0.01 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "serial.h"



/********************************************************************************************************************
*   Documented in `serial.h`
*///-----------------------------------------------------------------------------------------------------------------
void SerialOpen(void)
{
    OUTB(COM1_BASE + 1, 0x00);       // Disable all interrupts
    OUTB(COM1_BASE + 3, 0x80);       // Enable DLAB (set baud rate divisor)
    OUTB(COM1_BASE + 0, 0x01);       // Set divisor to 1 (lo byte) 115200 baud
    OUTB(COM1_BASE + 1, 0x00);       //                  (hi byte)
    OUTB(COM1_BASE + 3, 0x03);       // 8 bits, no parity, one stop bit
    OUTB(COM1_BASE + 2, 0xC7);       // Enable FIFO, clear them, with 14-byte threshold
    OUTB(COM1_BASE + 4, 0x0B);       // IRQs enabled, RTS/DSR set
}



/********************************************************************************************************************
*   Documented in `serial.h`
*///-----------------------------------------------------------------------------------------------------------------
void SerialPutChar(uint8_t ch)
{
    if (ch == '\n') SerialPutChar('\r');

    while ((INB(COM1_BASE + 5) & 0x20) == 0) {}

    OUTB(COM1_BASE + 0, ch);
}


