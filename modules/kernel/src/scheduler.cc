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


//#define USE_SERIAL

#include "types.h"
#include "heap.h"
#include "cpu.h"
#include "stacks.h"
#include "lists.h"
#include "kernel-funcs.h"
#include "boot-interface.h"
#include "scheduler.h"



//
// -- Some local function prototypes
//    ------------------------------
extern "C" {
    void ProcessUpdateTimeUsed(void);

    void ProcessLockAndPostpone(void);
    void ProcessUnlockAndSchedule(void);
    void ProcessLockScheduler(bool save);
    void ProcessUnlockScheduler(void);
}



//
// -- This is the scheduler object
//    ----------------------------
Scheduler_t scheduler = {
    0,                      // nextPID
    ~((Addr_t)0),           // nextWake
    false,                  // processChangePending
    0,                      // flags
    {0},                    // schedulerLockCount
    {0},                    // postponeCount
    -1,                     // lock CPU
    {{0}},                  // the os ready queue
    {{0}},                  // the high ready queue
    {{0}},                  // the normal ready queue
    {{0}},                  // the low ready queue
    {{0}},                  // the idle ready queue
    {{0}},                  // the list of blocked processes
    {{0}},                  // the list of sleeping processes
    {{0}},                  // the list of terminated tasks
    {{0}},                  // the global process list
};


Spinlock_t schedulerLock = {0};


#ifdef USE_SERIAL
//
// -- Convert a ProcStatus_t to a string
//    -----------------------------------
static const char *ProcStatusStr(ProcStatus_t s) {
    if (s == PROC_INIT)         return "INIT";
    else if (s == PROC_RUNNING) return "RUNNING";
    else if (s == PROC_READY)   return "READY";
    else if (s == PROC_TERM)    return "TERM";
    else if (s == PROC_MTXW)    return "MTXW";
    else if (s == PROC_SEMW)    return "SEMW";
    else if (s == PROC_DLYW)    return "DLYW";
    else if (s == PROC_MSGW)    return "MSGW";
    else                        return "Unknown!";
}



//
// -- Convert a ProcStatus_t to a string
//    -----------------------------------
static const char *ProcPriorityStr(ProcPriority_t p) {
    if (p == PTY_IDLE)          return "IDLE";
    else if (p == PTY_LOW)      return "LOW";
    else if (p == PTY_NORM)     return "NORMAL";
    else if (p == PTY_HIGH)     return "HIGH";
    else if (p == PTY_OS)       return "OS";
    else                        return "Unknown!";
}
#endif



//
// -- Dump a Process structure for debugging purposes
//    -----------------------------------------------
void DumpProcess(Process_t *proc)
{
#ifdef USE_SERIAL
    kprintf("=========================================================\n");
    kprintf("Dumping Process_t structure at address %p\n", proc);

    if (!proc) goto exit;

    kprintf("---------------------------------------------------------\n");
    kprintf("  TOS (last preemption)..: %p\n", proc->tosProcessSwap);
    kprintf("  Virtual Address Space..: %p\n", proc->virtAddrSpace);
    kprintf("  Process Status.........: %d (%s)\n", proc->status, ProcStatusStr(proc->status));
    kprintf("  Process Priority.......: %d (%s)\n", proc->priority, ProcPriorityStr(proc->priority));
    kprintf("  Quantum left this slice: %d\n", AtomicRead(&proc->quantumLeft));
    kprintf("  Process ID.............: %d\n", proc->pid);
    kprintf("  Command Line...........: %s\n", proc->command);
    kprintf("  Micros used............: %lu\n", proc->timeUsed);
    kprintf("  Wake tick number.......: %d\n", proc->wakeAtMicros);
    kprintf("  Pending Error Number...: %d\n", proc->pendingErrno);

exit:
    kprintf("=========================================================\n");
#endif
}



//
// -- Dump the current status of the Ready Queue
//    ------------------------------------------
void DumpReadyQueue(void)
{
#ifdef USE_SERIAL
    ListHead_t::List_t *wrk = NULL;
    kprintf("=========================================================\n");
    kprintf("Dumping OS Priority Ready Queue\n");
    kprintf("---------------------------------------------------------\n");

    wrk = scheduler.queueOS.list.next;
    while (wrk != &scheduler.queueOS.list) {
        Process_t *proc = FIND_PARENT(wrk, Process_t, stsQueue);
        kprintf("  Process %s at %p\n", proc->command, proc);
        wrk = wrk->next;
    }

    kprintf("\nDumping High Priority Ready Queue\n");
    kprintf("---------------------------------------------------------\n");

    wrk = scheduler.queueHigh.list.next;
    while (wrk != &scheduler.queueHigh.list) {
        Process_t *proc = FIND_PARENT(wrk, Process_t, stsQueue);
        kprintf("  Process %s at %p\n", proc->command, proc);
        wrk = wrk->next;
    }

    kprintf("\nDumping Normal Priority Ready Queue\n");
    kprintf("---------------------------------------------------------\n");

    wrk = scheduler.queueNormal.list.next;
    while (wrk != &scheduler.queueNormal.list) {
        Process_t *proc = FIND_PARENT(wrk, Process_t, stsQueue);
        kprintf("  Process %s at %p\n", proc->command, proc);
        wrk = wrk->next;
    }

    kprintf("\nDumping Low Priority Ready Queue\n");
    kprintf("---------------------------------------------------------\n");

    wrk = scheduler.queueLow.list.next;
    while (wrk != &scheduler.queueLow.list) {
        Process_t *proc = FIND_PARENT(wrk, Process_t, stsQueue);
        kprintf("  Process %s at %p\n", proc->command, proc);
        wrk = wrk->next;
    }

    kprintf("\nDumping idle Priority Ready Queue\n");
    kprintf("---------------------------------------------------------\n");

    wrk = scheduler.queueIdle.list.next;
    while (wrk != &scheduler.queueIdle.list) {
        Process_t *proc = FIND_PARENT(wrk, Process_t, stsQueue);
        kprintf("  Process %s at %p\n", proc->command, proc);
        wrk = wrk->next;
    }

    kprintf("\nSome other interesting information\n");
    kprintf("---------------------------------------------------------\n");
    kprintf("  Scheduler Process Change Pending: %s\n", scheduler.processChangePending?"yes":"no");
    kprintf("  Scheduler Lock Count: %d\n", AtomicRead(&scheduler.schedulerLockCount));
    kprintf("  Scheduler Postpone Count: %d\n", AtomicRead(&scheduler.postponeCount));

    kprintf("=========================================================\n");
#endif
}



//
// -- Idle when there is nothing to do
//    --------------------------------
static void ProcessIdle(void)
{
    kprintf("Starting the idle process\n");
    CurrentThread()->priority = PTY_IDLE;

    while (true) {
        assert(CurrentThread()->status == PROC_RUNNING);
        EnableInt();
        __asm volatile ("hlt" ::: "memory");
    }
}



//
// -- Lock the scheduler in preparation for changes
//    ---------------------------------------------
void ProcessLockScheduler(bool save)
{
    if (AtomicRead(&scheduler.schedulerLockCount) == 0 || scheduler.lockCpu != ThisCpu()->cpuNum) {
        Addr_t flags = DisableInt();
        SpinLock(&schedulerLock);

        scheduler.lockCpu = ThisCpu()->cpuNum;

        if (save) scheduler.flags = flags;
    }

    AtomicInc(&scheduler.schedulerLockCount);
}



//
// -- Scheduler locking, postponing, unlocking, and scheduling functions
//    ------------------------------------------------------------------
void ProcessLockAndPostpone(void)
{
    ProcessLockScheduler(true);
    AtomicInc(&scheduler.postponeCount);
}



//
// -- Unlock the scheduler after changes
//    ----------------------------------
void ProcessUnlockScheduler(void)
{
    assert_msg(AtomicRead(&scheduler.schedulerLockCount) > 0, "schedulerLockCount out if sync");

    if (AtomicDecAndTest0(&scheduler.schedulerLockCount)) {
        SpinUnlock(&schedulerLock);
        RestoreInt(scheduler.flags);
    }
}



//
// -- decrease the lock count on the scheduler
//    ----------------------------------------
void ProcessUnlockAndSchedule(void)
{
    assert_msg(AtomicRead(&scheduler.postponeCount) > 0, "postponeCount out if sync");
    assert_msg(!(AtomicRead(&scheduler.postponeCount) < 0), "postponeCount is negative");

    if (AtomicDecAndTest0(&scheduler.postponeCount) == true) {
        if (scheduler.processChangePending != false) {
            scheduler.processChangePending = false;           // need to clear this to actually perform a change
            ProcessSchedule();
        }
    }

    ProcessUnlockScheduler();
}



//
// -- Find the next process to give the CPU to
//    ----------------------------------------
static Process_t *ProcessNext(ProcPriority_t pty)
{
    DumpReadyQueue();

    if (IsListEmpty(&scheduler.queueOS) == false) {
        return FIND_PARENT(scheduler.queueOS.list.next, Process_t, stsQueue);
    } else if (IsListEmpty(&scheduler.queueHigh) == false && PTY_HIGH >= pty) {
        return FIND_PARENT(scheduler.queueHigh.list.next, Process_t, stsQueue);
    } else if (IsListEmpty(&scheduler.queueNormal) == false && PTY_NORM >= pty) {
        return FIND_PARENT(scheduler.queueNormal.list.next, Process_t, stsQueue);
    } else if (IsListEmpty(&scheduler.queueLow) == false && PTY_LOW >= pty) {
        return FIND_PARENT(scheduler.queueLow.list.next, Process_t, stsQueue);
    } else if (IsListEmpty(&scheduler.queueIdle) == false && PTY_IDLE >= pty) {
        return FIND_PARENT(scheduler.queueIdle.list.next, Process_t, stsQueue);
    } else {
//        kprintf(".. no next process\n");
        return NULL;
    }
}



//
// -- Get the current timer value and update the time used of the current process
//    ---------------------------------------------------------------------------
void ProcessUpdateTimeUsed(void)
{
    uint64_t now = TmrCurrentCount();
    uint64_t elapsed = now - ThisCpu()->lastTimer;
    ThisCpu()->lastTimer = now;

    if (CurrentThread() == NULL) {
        ThisCpu()->cpuIdleTime += elapsed;
    } else {
        CurrentThread()->timeUsed += elapsed;
    }
}



//
// -- Add a process to the global process List
//    ----------------------------------------
static void ProcessAddGlobal(Process_t *proc)
{
    ProcessLockAndPostpone();

    kprintf(".. Checking scheduler Global Process List: %p (%p)\n", &scheduler.globalProcesses, scheduler.globalProcesses);
    ListAddTail(&scheduler.globalProcesses, &proc->globalList);

    ProcessUnlockAndSchedule();
}



//
// -- Remove the process from whatever list it is on, ensuring proper locking
//    -----------------------------------------------------------------------
static void ProcessListRemove(Process_t *proc)
{
    DumpReadyQueue();
    if (!assert(proc != NULL)) return;


    // -- is it already not on a list?
    if (proc->stsQueue.next == &proc->stsQueue) return;


    //
    // -- Is this process on a queue?
    //    ---------------------------
    if (proc->status != PROC_RUNNING) {
        switch (proc->status) {
        case PROC_DLYW:
        case PROC_MSGW:
        case PROC_MTXW:
        case PROC_SEMW:
        case PROC_TERM:
            ListRemoveInit(&proc->stsQueue);
            break;

        case PROC_READY:
            switch (proc->priority) {
            case PTY_OS:
                ListRemoveInit(&proc->stsQueue);
                break;

            case PTY_HIGH:
                ListRemoveInit(&proc->stsQueue);
                break;

            case PTY_LOW:
                ListRemoveInit(&proc->stsQueue);
                break;

            default:
                ListRemoveInit(&proc->stsQueue);
                break;
            }

            break;

        default:
            // do nothing
            break;
        }
    }
}



//
// -- Output the state of the scheduler
//    ---------------------------------
void ProcessCheckQueue(void)
{
    ProcessLockAndPostpone();
    kprintf("Dumping the status of the scheduler on CPU%d\n", ThisCpu()->cpuNum);
    kprintf("The scheduler is %s\n", schedulerLock.lock?"locked":"unlocked");
    if (schedulerLock.lock) kprintf("...  on CPU%d\n", scheduler.lockCpu);
    assert(schedulerLock.lock != 0);
    assert(scheduler.lockCpu == ThisCpu()->cpuNum);
    kprintf(".. postpone count %d\n", AtomicRead(&scheduler.postponeCount));
    kprintf(".. currently, a reschedule is %spending\n", scheduler.processChangePending ? "" : "not ");
    kprintf("..     OS Queue process count: %d\n", ListCount(&scheduler.queueOS));
    kprintf("..   High Queue process count: %d\n", ListCount(&scheduler.queueHigh));
    kprintf(".. Normal Queue process count: %d\n", ListCount(&scheduler.queueNormal));
    kprintf("..    Low Queue process count: %d\n", ListCount(&scheduler.queueLow));
    kprintf("..   Idle Queue process count: %d\n", ListCount(&scheduler.queueIdle));
    kprintf(".. There are %d processes on the terminated list\n", ListCount(&scheduler.listTerminated));
    ProcessUnlockAndSchedule();
}



//
// -- pick the next process to execute and execute it
//
//    CRITICAL: ProcessLockScheduler() must be called before calling
//    --------------------------------------------------------------
void ProcessSchedule(void)
{
    assert_msg(AtomicRead(&scheduler.schedulerLockCount) > 0,
            "Calling `ProcessSchedule()` without holding the proper lock");
    if (!assert(CurrentThread() != NULL)) {
        kprintf("FATAL: currentThread is NULL entering ProcessSchedule");
        while (true) {
            __asm volatile ("cli");
            __asm volatile ("hlt");
        }
    }

    Process_t *next = NULL;

    if (AtomicRead(&scheduler.postponeCount) != 0) {
        scheduler.processChangePending = true;
        return;
    }

    next = ProcessNext(CurrentThread()?CurrentThread()->priority:PTY_IDLE);

    if (next != NULL) {
        ProcessListRemove(next);
        assert(AtomicRead(&scheduler.postponeCount) == 0);
        ProcessSwitch(next);
    } else if (CurrentThread()->status == PROC_RUNNING) {
        // -- Do nothing; the current process can continue; reset quantum
        AtomicAdd(&(CurrentThread()->quantumLeft), CurrentThread()->priority);
        return;
    } else {
        // -- No tasks available; so we go into idle mode
        Process_t *save = CurrentThread();          // we will save this process for later
        CurrentThreadAssign(NULL);                  // nothing is running!

        do {
            // -- -- temporarily unlock the scheduler and enable interrupts for the timer to fire
            ProcessUnlockScheduler();
            EnableInt();
            __asm volatile("hlt" ::: "memory");
            DisableInt();
            ProcessLockScheduler(false);     // make sure that this does not overwrite the process's flags
            next = ProcessNext(PTY_IDLE);
        } while (next == NULL);
        ProcessListRemove(next);

        // -- restore the current Process and change if needed
        ProcessUpdateTimeUsed();
        CurrentThreadAssign(save);
        AtomicSet(&next->quantumLeft, next->priority);

        if (next != CurrentThread()) ProcessSwitch(next);

    }
}



//
// -- Terminate a task
//    ----------------
void ProcessTerminate(Process_t *proc)
{
    if (!assert(proc != NULL)) return;

    ProcessLockAndPostpone();

    if (proc == CurrentThread()) {
        assert(proc->stsQueue.next == &proc->stsQueue);
        Enqueue(&scheduler.listTerminated, &proc->stsQueue);
        sch_ProcessBlock(0, PROC_TERM);
    } else {
        kprintf(".. termianting another process\n");
        ProcessListRemove(proc);
        Enqueue(&scheduler.listTerminated, &proc->stsQueue);
        proc->status = PROC_TERM;
    }

    ProcessUnlockAndSchedule();
}



//
// -- complete any new task initialization
//    ------------------------------------
void ProcessStart(void)
{
    assert_msg(AtomicRead(&scheduler.schedulerLockCount) > 0,
            "`ProcessStart()` is executing for a new process without holding the proper lock");
    assert_msg(AtomicRead(&scheduler.schedulerLockCount) == 1,
            "`ProcessStart()` is executing while too many locks are held");

    kprintf("Starting new process with address space %p\n", GetAddressSpace());

    ProcessUnlockScheduler();

    assert_msg(AtomicRead(&scheduler.schedulerLockCount) == 0,
            "`ProcessStart()` still has a scheduler lock remaining");

    assert_msg(AtomicRead(&scheduler.postponeCount) == 0, "`ProcessStart()` with a pending process change");

    EnableInt();
}


//
// -- End current process
//    -------------------
void ProcessEnd(void)
{
    ProcessLockAndPostpone();

    Process_t *proc = CurrentThread();
    assert(proc->stsQueue.next == &proc->stsQueue);
    Enqueue(&scheduler.listTerminated, &proc->stsQueue);
    sch_ProcessBlock(0, PROC_TERM);

    // -- send a message with the scheduler already locked
//    _MessageQueueSend(butlerMsgq, BUTLER_CLEAN_PROCESS, 0, 0, false);

    ProcessUnlockAndSchedule();
}




//
// -- The scheduler timer resccheduling function
//    ------------------------------------------
Return_t sch_Tick(int, uint64_t now)
{
    kprintf("*");
    ProcessLockAndPostpone();


    //
    // -- here we look for any sleeping tasks to wake
    //    -------------------------------------------
    if (now >= scheduler.nextWake && IsListEmpty(&scheduler.listSleeping) == false) {
        uint64_t newWake = (uint64_t)-1;


        //
        // -- loop through and find the processes to wake up
        //    ----------------------------------------------
        ListHead_t::List_t *list = scheduler.listSleeping.list.next;
        while (list != &scheduler.listSleeping.list) {
            ListHead_t::List_t *next = list->next;      // must be saved before it is changed below
            Process_t *wrk = FIND_PARENT(list, Process_t, stsQueue);
            if (now >= wrk->wakeAtMicros) {
                wrk->wakeAtMicros = 0;
                ListRemoveInit(&wrk->stsQueue);
                sch_ProcessUnblock(0, wrk);
            } else if (wrk->wakeAtMicros < newWake) newWake = wrk->wakeAtMicros;

            list = next;
        }

        scheduler.nextWake = newWake;
    }


    //
    // -- adjust the quantum and see if it is time to change tasks
    //    --------------------------------------------------------
    if (CurrentThread() != NULL) {
        AtomicDec(&(CurrentThread()->quantumLeft));
        if (AtomicRead(&CurrentThread()->quantumLeft) <= 0) {
            scheduler.processChangePending = true;
        }
    }

    ProcessUnlockAndSchedule();

    return 0;
}


//
// -- Create a new process structure and leave it on the Ready Queue
//    --------------------------------------------------------------
Process_t *sch_ProcessCreate(int, const char *name, Addr_t startingAddr, Addr_t addrSpace)
{
    kprintf("Creating a new process named at %p (%s), starting at %p\n", name, name, startingAddr);
    kprintf(".. the address space for this process in %p\n", addrSpace);

    Process_t *rv = NEW(Process_t);
    if (!assert_msg(rv != NULL, "Out of memory allocating a new Process_t")) {
        kprintf("Out of memory allocating a new Process_t");
        while (true) {
            __asm volatile("hlt");
        }
    }

    kMemSetB(rv, 0, sizeof(Process_t));

    // -- set the name of the process
    kprintf(".. naming the process: %s\n", name);
    int len = kStrLen(name + 1);
    rv->command[len + 1] = 0;
    kStrCpy(rv->command, name);


    rv->policy = POLICY_0;
    rv->priority = PTY_OS;
    rv->status = PROC_INIT;
    AtomicSet(&rv->quantumLeft, rv->priority);
    rv->timeUsed = 0;
    rv->wakeAtMicros = 0;
    ListInit(&rv->stsQueue);
    ListInit(&rv->references.list);
    ListInit(&rv->globalList);


    //
    // -- Construct the stack for the architecture
    //    ----------------------------------------
    kprintf(".. Creating the new Process's stack\n");
    rv->virtAddrSpace = addrSpace;
    ProcessNewStack(rv, startingAddr);


    //
    // -- Put this process on the queue to execute
    //    ----------------------------------------
    kprintf(".. Readying the new process to be scheduled: %p\n", rv);
    DumpProcess(rv);
    sch_ProcessReady(0, rv);


    return rv;
}


//
// -- Initialize the process structures
//    ---------------------------------
Return_t ProcessInit(BootInterface_t *loaderInterface)
{
    ProcessInitTable();

    // -- map/unmap this address to build out the intermediate tables
    MmuMapPage(MMU_STACK_INIT_VADDR, 1, 0);
    MmuUnmapPage(MMU_STACK_INIT_VADDR);

    kprintf("ProcessInit() called\n");

    kprintf("Process table offsets:\n");
    kprintf("  TOS: %d\n", offsetof(Process_t, tosProcessSwap));
    kprintf("  Virtual Address Space: %d\n", offsetof(Process_t, virtAddrSpace));
    kprintf("  Status: %d\n", offsetof(Process_t, status));
    kprintf("  Priority: %d\n", offsetof(Process_t, priority));
    kprintf("  Quantum Left: %d\n", offsetof(Process_t, quantumLeft));
    kprintf("Scheduler structure offsets:\n");
    kprintf("  Change pending: %d (%d)\n", offsetof(Scheduler_t, processChangePending), sizeof(bool));
    kprintf("  Lock count: %d (%d)\n", offsetof(Scheduler_t, schedulerLockCount), sizeof (AtomicInt_t));
    kprintf("  Postpone count: %d (%d)\n", offsetof(Scheduler_t, postponeCount), sizeof(AtomicInt_t));


    ListInit(&scheduler.queueOS.list);
    ListInit(&scheduler.queueHigh.list);
    ListInit(&scheduler.queueNormal.list);
    ListInit(&scheduler.queueLow.list);
    ListInit(&scheduler.queueIdle.list);
    ListInit(&scheduler.listBlocked.list);
    ListInit(&scheduler.listSleeping.list);
    ListInit(&scheduler.listTerminated.list);
    ListInit(&scheduler.globalProcesses.list);

    Process_t *proc = NEW(Process_t);

    kprintf(".. the current process is located at %p\n", proc);

    if (!assert(proc != NULL)) {
        kprintf("FATAL: Unable to allocate Current Process structure\n");

        while (true) {
            __asm volatile ("hlt");
        }
    }

    kMemSetB(proc, 0, sizeof(Process_t));

    proc->tosProcessSwap = 0;
    proc->virtAddrSpace = loaderInterface->bootVirtAddrSpace;
    proc->pid = scheduler.nextPID ++;          // -- this is the butler process ID


    // -- set the process name
    kMemSetB(proc->command, 0, CMD_LEN);
    kStrCpy(proc->command, "kInit");

    proc->policy = POLICY_0;
    proc->priority = PTY_OS;
    proc->status = PROC_RUNNING;
    AtomicSet(&proc->quantumLeft, PTY_OS);
    proc->timeUsed = 0;
    proc->wakeAtMicros = 0;
    ListInit(&proc->stsQueue);
    ListInit(&proc->references.list);
    ProcessAddGlobal(proc);           // no lock required -- still single threaded
    CurrentThreadAssign(proc);

    kprintf("Process Init:\n");
    DumpProcess(proc);


    //
    // -- Create an idle process for each CPU
    //    -----------------------------------
    for (int i = 0; i < loaderInterface->cpuCount; i ++) {
        kprintf("starting idle process %d\n", i, ProcessIdle);
        sch_ProcessCreate(0, "Idle Process", (Addr_t)ProcessIdle, GetAddressSpace());
    }


    ThisCpu()->lastTimer = TmrCurrentCount();
    kprintf("ProcessInit() complete\n");

    return 0;
}



//
// -- Functions to block the current process
//    --------------------------------------
Return_t sch_ProcessBlock(int, ProcStatus_t reason)
{
    if (!assert(reason >= PROC_INIT && reason <= PROC_MSGW)) return -EINVAL;
    if (!assert(CurrentThread() != NULL)) return -EINVAL;

//    kprintf("Blocking current process %p\n", CurrentThread());

    ProcessLockAndPostpone();
    CurrentThread()->status = reason;
    CurrentThread()->pendingErrno = 0;
    ProcessSchedule();

    ProcessUnlockAndSchedule();

    return 0;
}



//
// -- Place a process on the correct ready queue
//    ------------------------------------------
Return_t sch_ProcessReady(int, Process_t *proc)
{
    if (!assert(proc != NULL)) return -EINVAL;

    ProcessLockAndPostpone();

    if (proc->globalList.next == &proc->globalList) {
        proc->pid = scheduler.nextPID ++;

        proc->status = PROC_READY;

        ProcessAddGlobal(proc);
    }

    proc->status = PROC_READY;

    switch(proc->priority) {
    case PTY_OS:
        Enqueue(&scheduler.queueOS, &proc->stsQueue);
        break;

    case PTY_HIGH:
        Enqueue(&scheduler.queueHigh, &proc->stsQueue);
        break;

    default:
        // in this case, we have a priority that is not right; assume normal from now on
        proc->priority = PTY_NORM;
        // ...  fall through

    case PTY_NORM:
        Enqueue(&scheduler.queueNormal, &proc->stsQueue);
        break;

    case PTY_LOW:
        Enqueue(&scheduler.queueLow, &proc->stsQueue);
        break;

    case PTY_IDLE:
        Enqueue(&scheduler.queueIdle, &proc->stsQueue);
        break;
    }

    ProcessUnlockAndSchedule();

    return 0;
}



//
// -- Unblock a process
//    -----------------
Return_t sch_ProcessUnblock(int, Process_t *proc)
{
    if (!assert(proc != NULL)) return -EINVAL;

    ProcessLockAndPostpone();
    proc->status = PROC_READY;
    sch_ProcessReady(0, proc);
    ProcessUnlockAndSchedule();

    return 0;
}



//
// -- Sleep until the we reach the number of micro-seconds since boot
//    ---------------------------------------------------------------
Return_t sch_ProcessMicroSleepUntil(int, uint64_t when)
{
    if (when <= TmrCurrentCount()) return 0;

    ProcessLockAndPostpone();
    CurrentThread()->wakeAtMicros = when;
    if (when < scheduler.nextWake) scheduler.nextWake = when;
    Enqueue(&scheduler.listSleeping, &(CurrentThread()->stsQueue));
    sch_ProcessBlock(0, PROC_DLYW);
    ProcessUnlockAndSchedule();

    return 0;
}



