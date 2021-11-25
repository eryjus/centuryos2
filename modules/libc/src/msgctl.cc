/****************************************************************************************************************//**
*   @file               msgctl.cc
*   @brief              The `msgctl` system call
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
int msgctl(int msqid, int cmd, struct msqid_ds *buf)
{
    int rv = 0;

    struct msqid_ds *b = (struct msqid_ds *)KrnCopyMem(buf, sizeof(struct msqid_ds));

    // execute the syscall
    __asm volatile ("                   \
        mov     %1,%%rdi\n              \
        mov     %2,%%rsi\n              \
        mov     %3,%%rdx\n              \
        mov     %4,%%rcx\n              \
        int     %5\n                    \
        mov     %%rax,%0\n              \
    "   : "=m"(rv)
        : "N"(SVC_MSGCTL), "m"(msqid), "m"(cmd), "m"(b), "N"(OS_SERVICE_INT)
        : "rdi", "rsi", "rdx", "rcx", "rax", "cc", "memory");


    kMemMoveB(buf, b, sizeof(struct msqid_ds));
    KrnReleaseMem(b);

    if (rv < 0) {
        errno = -rv;
        return -1;
    } else return rv;
}

