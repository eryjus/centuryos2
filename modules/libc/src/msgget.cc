/****************************************************************************************************************//**
*   @file               msgget.cc
*   @brief              The `msgget` system call
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Nov-20
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
*   | 2021-Nov-20 | Initial |  v0.0.14 | ADCL | Initial version
*
*///=================================================================================================================



#include "constants.h"
#include "errno.h"
#include "kernel-funcs.h"
#include <sys/msg.h>



/********************************************************************************************************************
*   documented in `sys/msg.h`
*///-----------------------------------------------------------------------------------------------------------------
int msgget(key_t key, int msgflg)
{
    int rv = 0;

    // execute the syscall
    __asm volatile ("                   \
        mov     %1,%%rdi\n              \
        mov     %2,%%rsi\n              \
        mov     %3,%%rdx\n              \
        int     %4\n                    \
        mov     %%rax,%0\n              \
    "   : "=m"(rv)
        : "N"(SVC_MSGGET), "m"(key), "m"(msgflg), "N"(OS_SERVICE_INT)
        : "rdi", "rsi", "rdx", "rax", "cc", "memory");


    if (rv < 0) {
        errno = -rv;
        return -1;
    } else return rv;
}


