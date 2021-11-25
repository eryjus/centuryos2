/****************************************************************************************************************//**
*   @file               msgrcv.cc
*   @brief              The `msgrcv` system call
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
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
    ssize_t rv = 0;

    void *m = KrnCopyMem(msgp, sizeof(long) + msgsz);

    // execute the syscall
    __asm volatile ("                   \
        mov     %1,%%rdi\n              \
        mov     %2,%%rsi\n              \
        mov     %3,%%rdx\n              \
        mov     %4,%%rcx\n              \
        mov     %5,%%r8\n               \
        mov     %6,%%r9\n               \
        int     %7\n                    \
        mov     %%rax,%0\n              \
    "   : "=m"(rv)
        : "N"(SVC_MSGRCV), "m"(msqid), "m"(m), "m"(msgsz), "m"(msgtyp), "m"(msgflg), "N"(OS_SERVICE_INT)
        : "rdi", "rsi", "rdx", "rcx", "r8", "r9", "rax", "cc", "memory");

    kMemMoveB((void *)msgp, m, msgsz);
    KrnReleaseMem(m);

    if (rv < 0) {
        errno = -rv;
        return -1;
    } else return rv;
}

