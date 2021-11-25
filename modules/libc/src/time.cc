/****************************************************************************************************************//**
*   @file               time.cc
*   @brief              The `time` system call
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Nov-21
*   @since              v0.0.14
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Nov-21 | Initial |  v0.0.14 | ADCL | Initial version
*
*///=================================================================================================================



#include "constants.h"
#include "kernel-funcs.h"

#include <time.h>



/********************************************************************************************************************
*   documented in `time.h`
*///-----------------------------------------------------------------------------------------------------------------
time_t time(time_t *t)
{
    time_t rv = 0;
    time_t *_t = t;             // will get clobbered by the system call below.

    // execute the syscall
    __asm volatile ("                   \
        mov     %1,%%rdi\n              \
        int     %2\n                    \
        mov     %%rax,%0\n              \
    " : "=m"(rv) : "N"(SVC_RTCTIME), "N"(OS_SERVICE_INT) : "rax", "rdi", "cc", "memory");

    if (_t) *_t = rv;
    return rv;
}


