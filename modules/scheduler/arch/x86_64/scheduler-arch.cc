//===================================================================================================================
//
//  scheduler-arch.cc -- Architecture-specific functions elements for the process
//
//        Copyright (c)  2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-May-26  Initial  v0.0.9b  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "stacks.h"
#include "kernel-funcs.h"
#include "heap.h"
#include "scheduler.h"



//
// -- This is the lock to get permission to map the stack for initialization
//    ----------------------------------------------------------------------
Spinlock_t mmuStackInitLock = {0};


//
// -- build the stack needed to start a new process
//    ---------------------------------------------
void ProcessNewStack(Process_t *proc, void (*startingAddr)(void))
{
    Addr_t *stack;
    Frame_t *stackFrames;
    const size_t frameCount = STACK_SIZE / PAGE_SIZE;

    stackFrames = (Frame_t *)HeapAlloc(sizeof (Frame_t *) * frameCount, false);
    for (int i = 0; i < frameCount; i ++) {
        stackFrames[i] = PmmAlloc();
    }

    Addr_t flags = DisableInt();
    SpinLock(&mmuStackInitLock); {
        for (int i = 0; i < frameCount; i ++) {
            MmuMapPage(MMU_STACK_INIT_VADDR + (PAGE_SIZE * i), stackFrames[i], true);
        }

        stack = (Addr_t *)(MMU_STACK_INIT_VADDR + STACK_SIZE);

//        *--stack = ProcessEnd;               // -- just in case, we will self-terminate
        *--stack = (Addr_t)startingAddr;       // -- this is the process starting point
        *--stack = (Addr_t)ProcessStart;       // -- initialize a new process
        *--stack = 0;                          // -- ebx
        *--stack = 0;                          // -- esi
        *--stack = 0;                          // -- edi
        *--stack = 0;                          // -- ebp

        for (int i = 0; i < frameCount; i ++) {
            MmuUnmapPage(MMU_STACK_INIT_VADDR + (PAGE_SIZE * i));
        }

        SpinUnlock(&mmuStackInitLock);
        RestoreInt(flags);
    }

    Addr_t stackLoc = StackFind();    // get a new stack
    assert(stackLoc != 0);
    proc->tosProcessSwap = ((Addr_t)stack - MMU_STACK_INIT_VADDR) + stackLoc;

    for (int i = 0; i < frameCount; i ++) {
        MmuMapPage(stackLoc + (PAGE_SIZE * i), stackFrames[i], true);
    }
}


