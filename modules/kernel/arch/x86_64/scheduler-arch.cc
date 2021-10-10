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
void ProcessNewStack(Process_t *proc, Addr_t startingAddr)
{
    Addr_t *stack;
    Frame_t *stackFrames = NULL;
    const size_t frameCount = STACK_SIZE / PAGE_SIZE;

    KernelPrintf("Creating a new Process stack\n");
    KernelPrintf(".. allocating frames for the stack\n");
    stackFrames = (Frame_t *)HeapAlloc(sizeof (Frame_t *) * frameCount, false);
    assert_msg(stackFrames != NULL, "HeapAlloc() ran out of memory allocating stack frames!");
    KernelPrintf(".. Temporary Stack Frames are located at %p\n", stackFrames);

    for (int i = 0; i < frameCount; i ++) {
        KernelPrintf(".. allocating stack frame %d of %d\n", i + 1, frameCount);
        stackFrames[i] = PmmAlloc();
    }

    KernelPrintf(".. Locking the stack build spinlock\n");
    Addr_t flags = DisableInt();
    SpinLock(&mmuStackInitLock); {
        KernelPrintf(".. Mapping the stack to the temporary build address\n");
        for (int i = 0; i < frameCount; i ++) {
            MmuMapPage(MMU_STACK_INIT_VADDR + (PAGE_SIZE * i), stackFrames[i], PG_WRT);
        }

        KernelPrintf(".. Building the stack contents\n");
        stack = (Addr_t *)(MMU_STACK_INIT_VADDR + STACK_SIZE);

        *--stack = (Addr_t)ProcessEnd;         // -- just in case, we will self-terminate
        *--stack = startingAddr;               // -- this is the process starting point
        *--stack = (Addr_t)ProcessStart;       // -- initialize a new process
        *--stack = 0;                          // -- rax
        *--stack = 0;                          // -- rbx
        *--stack = 0;                          // -- rcx
        *--stack = 0;                          // -- rdx
        *--stack = 0;                          // -- rsi
        *--stack = 0;                          // -- rdi
        *--stack = 0;                          // -- rbp
        *--stack = 0;                          // -- r8
        *--stack = 0;                          // -- r9
        *--stack = 0;                          // -- r10
        *--stack = 0;                          // -- r11
        *--stack = 0;                          // -- r12
        *--stack = 0;                          // -- r13
        *--stack = 0;                          // -- r14
        *--stack = 0;                          // -- r15

        KernelPrintf(".. Unmapping the stack from temporary address space\n");
        for (int i = 0; i < frameCount; i ++) {
            MmuUnmapPage(MMU_STACK_INIT_VADDR + (PAGE_SIZE * i));
        }

        KernelPrintf(".. Unlocking the spinlock\n");
        SpinUnlock(&mmuStackInitLock);
        RestoreInt(flags);
    }

    KernelPrintf(".. Finding a stack address\n");
    Addr_t stackLoc = StackFind();    // get a new stack
    assert(stackLoc != 0);
    proc->tosProcessSwap = ((Addr_t)stack - MMU_STACK_INIT_VADDR) + stackLoc;

    KernelPrintf("Mapping the stack into address space %p\n", GetAddressSpace());
    for (int i = 0; i < frameCount; i ++) {
        KernelPrintf(".. in space %p: frame %d (%p to %p)\n", proc->virtAddrSpace, i,
                stackLoc + (PAGE_SIZE * i), stackFrames[i]);
        MmuMapPageEx(proc->virtAddrSpace, stackLoc + (PAGE_SIZE * i), stackFrames[i], PG_WRT);
        KernelPrintf(".. page mapped in the other address space\n");
    }

    HeapFree(stackFrames);
}


