/****************************************************************************************************************//**
*   @file               msgsnd.cc
*   @brief              The `msgsnd` system call
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
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
{
    int rv = 0;

    void *m = KrnCopyMem(msgp, sizeof(long) + msgsz);

    // execute the syscall
    __asm volatile ("                   \
        mov     %1,%%rdi\n              \
        mov     %2,%%rsi\n              \
        mov     %3,%%rdx\n              \
        mov     %4,%%rcx\n              \
        mov     %5,%%r8\n               \
        int     %6\n                    \
        mov     %%rax,%0\n              \
    "   : "=m"(rv)
        : "N"(SVC_MSGSND), "m"(msqid), "m"(m), "m"(msgsz), "m"(msgflg), "N"(OS_SERVICE_INT)
        : "rdi", "rsi", "rdx", "rcx", "r8", "rax", "cc", "memory");

    KrnReleaseMem(m);

    if (rv < 0) {
        errno = -rv;
        return -1;
    } else return rv;
}

