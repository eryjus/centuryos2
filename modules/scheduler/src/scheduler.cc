//===================================================================================================================
//
//  scheduler.cc --Functions for the Process management
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This file contains the structures and prototypes needed to manage processes.  This file was copied from
//  Century32 and will need to be updated for this kernel.
//
//  The process structure will be allocated with the new process stack -- adding it to the overhead of the process
//  itself.  Since the stacks will be multiples of 4K, one extra page will be allocated for the process stucture
//  overhead.  This will be placed below the stack in a manner where a stack underflow will not impact this
//  structure unless something really goes horrible and deliberately wrong.  The scheduler will be able to check
//  that the stack is within bounds properly and kill the process if needed (eventually anyway).  This simple
//  decision will eliminate the need for process structures or the need to allocate a process from the heap.
//
//  There are several statuses for processes that should be noted.  They are:
//  * INIT -- the process is initializing and is not ready to run yet.
//  * RUN -- the process is in a runnable state.  In this case, the process may or may not be the current process.
//  * END -- the process has ended and the Butler process needs to clean up the structures, memory, IPC, locks, etc.
//  * MTXW -- the process is waiting for a mutex and is ineligible to run.
//  * SEMW -- the process is waiting for a semaphore and is ineligible to run.
//  * DLYW -- the process is waiting for a timed delay and is ineligible to run.
//  * MSGW -- the process is waiting for the delivery of a message and is ineligible to run.
//  * ZOMB -- the process has died at the OS level for some reason or violation and the Butler process is going to
//            clean it up.
//
//  Additionally, we are going to support the ability for a user to hold a process.  In this case, the process will
//  also be ineligible to run.  These held processes will be moved into another list which will maintain the
//  overall status of the process.
//
//  The process priorities will serve 2 functions.  It will 1) provide a sequence of what is eligibe to run and
//  when from a scheduler perspective.  It will also 2) provide the quantum duration for which a process is able to
//  use the CPU.  In this case, a higher priority process will be able use the CPU longer than a low priority
//  process.  Additionally, the Idle process is also the Butler process.  When there is something that needs to
//  be done, the Butler will artificially raise its CPU priority to be an OS process while it is completing this
//  work.  When complete the Butler will reduce its priority again.
//
//  Finally, threads will be supported not yet be supported at the OS level.  If needed, they will be added at a
//  later time.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-May-10  Initial  v0.0.9   ADCL  Initial version
//
//===================================================================================================================



#include "scheduler.h"







#if 0
void ProcessInit(void)
{
    Process_t *proc = NEW(Process_t);
    CurrentThreadAssign(proc);

    if (!assert(proc != NULL)) {
        CpuPanicPushRegs("Unable to allocate Current Process structure");
    }

    kMemSetB(proc, 0, sizeof(Process_t));

    proc->tosProcessSwap = 0;
    proc->virtAddrSpace = mmuLvl1Table;
    proc->pid = scheduler.nextPID ++;          // -- this is the butler process ID
    proc->ssProcFrame = STACK_LOCATION >> 12;

    archsize_t kStack = StackFind();
    proc->tosKernel = kStack + STACK_SIZE;
    proc->ssKernFrame = PmmAllocateFrame();
    MmuMapToFrame(kStack, proc->ssKernFrame, PG_KRN | PG_WRT);


    // -- set the process name
    proc->command = (char *)HeapAlloc(20, false);
    kMemSetB(proc->command, 0, 20);
    kStrCpy(proc->command, "kInit");

    proc->policy = POLICY_0;
    proc->priority = PTY_OS;
    proc->status = PROC_RUNNING;
    AtomicSet(&proc->quantumLeft, PTY_OS);
    proc->timeUsed = 0;
    proc->wakeAtMicros = 0;
    ListInit(&proc->stsQueue);
    ListInit(&proc->references.list);
    ProcessDoAddGlobal(proc);           // no lock required -- still single threaded
    CLEAN_SCHEDULER();
    CLEAN_PROCESS(proc);

    kprintf("ProcessInit() established the current process at %p for CPU%d\n", proc, thisCpu->cpuNum);


    //
    // -- Create an idle process for each CPU
    //    -----------------------------------
    for (int i = 0; i < cpus.cpusDiscovered; i ++) {
        // -- Note ProcessCreate() creates processes at the OS priority...
        kprintf("starting idle process %d\n", i);
        ProcessCreate("Idle Process", ProcessIdle);
    }


    thisCpu->lastTimer = TimerCurrentCount(timerControl);
    kprintf("ProcessInit() complete\n");
}


#endif