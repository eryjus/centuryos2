//====================================================================================================================
//
//  kernel-funcs.h -- prototypes for the kernel internal functions
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-16  Initial  v0.0.6   ADCL  Initial version (relocated)
//
//===================================================================================================================


#ifndef __KERNEL_FUNCS_H__
#define __KERNEL_FUNCS_H__
#include "lists.h"
#pragma once


#include "types.h"
#include "debugger.h"






//
// -- These are the interface functions
//    ---------------------------------
extern "C" {
    Return_t InternalDispatch0(int func);
    Return_t InternalDispatch1(int func, Addr_t p1);
    Return_t InternalDispatch2(int func, Addr_t p1, Addr_t p2);
    Return_t InternalDispatch3(int func, Addr_t p1, Addr_t p2, Addr_t p3);
    Return_t InternalDispatch4(int func, Addr_t p1, Addr_t p2, Addr_t p3, Addr_t p4);
    Return_t InternalDispatch5(int func, Addr_t p1, Addr_t p2, Addr_t p3, Addr_t p4, Addr_t p5);
    Return_t InternalVariable(int func, Addr_t p1, ...);
}


//
// -- Some additional runtime assertion checking; purposefully set up for use in conditions
//    -------------------------------------------------------------------------------------
extern "C" bool AssertFailure(const char *expr, const char *msg, const char *file, int line);



//
// -- Process the init table
//    ----------------------
extern "C" void ProcessInitTable(void);


//
// -- Some compiler hints
//    -------------------
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)


//
// -- asserts
//    -------
#ifdef assert
#   undef assert
#endif

#ifdef RELEASE
#   define assert(e) true
#   define assert_msg(e,m) true
#else
#   define assert(e) (likely((e)) ? true : AssertFailure(#e, NULL, __FILE__, __LINE__))
#   define assert_msg(e,m) (likely((e)) ? true : AssertFailure(#e, (m), __FILE__, __LINE__))
#endif



//
// -- common functions
//    ----------------
extern "C" {
    void kMemSetB(void *buf, uint8_t byt, size_t cnt);
    void kMemMoveB(void *dest, void *src, size_t cnt);
    void kStrCpy(char *dest, const char *src);
    int kStrCmp(const char *str1, const char *str2);
    size_t kStrLen(const char *str);
    char *ksprintf(char *, const char *, ...);
}



#define INTERNAL0(type,name,func)                                                                               \
    inline type name(void) {                                                                                    \
        return (type)InternalDispatch0(func);                                                                   \
    }

#define INTERNAL1(type,name,func,type1)                                                                         \
    inline type name(type1 p1) {                                                                                \
        return (type)InternalDispatch1(func, (Addr_t)p1);                                                       \
    }

#define INTERNAL2(type,name,func,type1,type2)                                                                   \
    inline type name(type1 p1, type2 p2) {                                                                      \
        return (type)InternalDispatch2(func, (Addr_t)p1, (Addr_t)p2);                                           \
    }

#define INTERNAL3(type,name,func,type1,type2,type3)                                                             \
    inline type name(type1 p1, type2 p2, type3 p3) {                                                            \
        return (type)InternalDispatch3(func, (Addr_t)p1, (Addr_t)p2, (Addr_t)p3);                               \
    }

#define INTERNAL4(type,name,func,type1,type2,type3,type4)                                                       \
    inline type name(type1 p1, type2 p2, type3 p3, type4 p4) {                                                  \
        return (type)InternalDispatch4(func, (Addr_t)p1, (Addr_t)p2, (Addr_t)p3, (Addr_t)p4);                   \
    }

#define INTERNAL5(type,name,func,type1,type2,type3,type4,type5)                                                 \
    inline type name(type1 p1, type2 p2, type3 p3, type4 p4, type5 p5) {                                        \
        return (type)InternalDispatch5(func, (Addr_t)p1, (Addr_t)p2, (Addr_t)p3, (Addr_t)p4, (Addr_t)p5);       \
    }


// ====================================================================
// == Start with the fundamental internal table management functions ==
// ====================================================================


//
// -- Function 0x000 -- Get an Internal Handler Function Address
//
//    Prototype: Return_t GetInternalHandler(int number);
//    ----------------------------------------------------------
INTERNAL1(Addr_t, GetInternalHandler, INT_GET_INTERNAL, int)


//
// -- Function 0x001 -- Set an Internal Handler Function Address
//
//    Prototype: Return_t SetInternalHandler(int number, Addr_t address, Addr_t addrSpace, Addr_t stack);
//    ---------------------------------------------------------------------------------------------------
INTERNAL4(Return_t, SetInternalHandler, INT_SET_INTERNAL, int, Addr_t, Addr_t, Addr_t)


//
// -- Function 0x002 -- Get an Interrupt Vector Handler Function Address
//
//    Prototype: Return_t GetVectorHandler(int number);
//    ------------------------------------------------------------------
INTERNAL1(Addr_t, GetVectorHandler, INT_GET_VECTOR, int)


//
// -- Function 0x003 -- Set an Interrupt Vector Handler Function Address
//
//    Prototype: Return_t SetVectorHandler(int number, Addr_t address, Addr_t addrSpace, Addr_t stack);
//    -------------------------------------------------------------------------------------------------
INTERNAL4(Return_t, SetVectorHandler, INT_SET_VECTOR, int, Addr_t, Addr_t, Addr_t)


//
// -- Function 0x004 -- Get an OS Service Handler Function Address
//
//    Prototype: Return_t GetServiceHandler(int number);
//    ------------------------------------------------------------
INTERNAL1(Addr_t, GetServiceHandler, INT_GET_SERVICE, int)


//
// -- Function 0x005 -- Set an OS Service Handler Function Address
//
//    Prototype: Return_t SetServiceHandler(int number, Addr_t address, Addr_t addrSpace, Addr_t stack);
//    --------------------------------------------------------------------------------------------------
INTERNAL4(Return_t, SetServiceHandler, INT_SET_SERVICE, int, Addr_t, Addr_t, Addr_t)


// =============================
// == Kernel output functions ==
// =============================


//
// -- Function 0x008 -- Handle the Kernel's limited version of printf()
//
//    Note: this function is implemented in assembly -- no macro here.
//    -----------------------------------------------------------------
extern "C" int KernelPrintf(const char *fmt, ...);



// ========================
// == Spinlock functions ==
// ========================


//
// -- Function 0x010 -- Lock a spinlock
//
//    Prototype: Return_t SpinLock(Spinlock_t *lock);
//    -----------------------------------------------
INTERNAL1(Return_t, SpinLock, INT_KRN_SPIN_LOCK, Spinlock_t *)


//
// -- Function 0x011 -- Try a spinlock; timing out after some timeout value
//
//    Note: The timeout is not yet implemented
//
//    Prototype: Return_t SpinTry(Spinlock_t *lock, size_t timeout);
//    --------------------------------------------------------------
INTERNAL2(Return_t, SpinTry, INT_KRN_SPIN_TRY, Spinlock_t *, size_t)


//
// -- Function 0x012 -- Unlock a spinlock
//
//    Prototype: Return_t SpinUnlock(Spinlock_t *lock);
//    -------------------------------------------------
INTERNAL1(Return_t, SpinUnlock, INT_KRN_SPIN_UNLOCK, Spinlock_t *)


// ==================================
// == MMU (Page Mapping) functions ==
// ==================================


//
// -- Function 0x018 -- Map a page in the current address space
//
//    Prototype: Return_t MmuMapPage(Addr_t addr, Frame_t frame, int flags);
//    ----------------------------------------------------------------------
#define PG_NONE     (0)
#define PG_WRT      (1<<0)
#define PG_KRN      (1<<1)
#define PG_DEV      (1<<15)
INTERNAL3(Return_t, MmuMapPage, INT_KRN_MMU_MAP, Addr_t, Frame_t, int)


//
// -- Function 0x019 -- Unmap a page in the current address space
//
//    Prototype: Return_t MmuUnmapPage(Addr_t addr);
//    -----------------------------------------------------------
INTERNAL1(Return_t, MmuUnmapPage, INT_KRN_MMU_UNMAP, Addr_t)


//
// -- Function 0x01a -- Check if a page is mapped in the current address space
//
//    Prototype: Return_t MmuIsMapped(Addr_t addr);
//    ------------------------------------------------------------------------
INTERNAL1(Return_t, MmuIsMapped, INT_KRN_MMU_IS_MAPPED, Addr_t)


//
// -- Function 0x01b -- Dump the page tables for an address in the current address space
//
//    Prototype: Return_t MmuDump(Addr_t addr);
//    ----------------------------------------------------------------------------------
INTERNAL1(Return_t, MmuDump, INT_KRN_MMU_DUMP, Addr_t)


//
// -- Function 0x01c -- Map a page in the specified address space
//
//    Prototype: Return_t MmuMapPageEx(Addr_t space, Addr_t addr, Frame_t frame, int flags);
//    --------------------------------------------------------------------------------------
INTERNAL4(Return_t, MmuMapPageEx, INT_KRN_MMU_MAP_EX, Addr_t, Addr_t, Frame_t, int)


//
// -- Function 0x01d -- Unmap a page in the specified address space
//
//    Prototype: Return_t MmuUnmapPageEx(Addr_t space, Addr_t addr);
//    --------------------------------------------------------------
INTERNAL2(Return_t, MmuUnmapPageEx, INT_KRN_MMU_UNMAP_EX, Addr_t, Addr_t)



// ==============================
// == Kernel Utility functions ==
// ==============================


//
// -- Function 0x020 -- Copy user-space memory into kernel memory
//
//    Prototype: void *KrnCopyMem(const void *mem, size_t size);
//    -----------------------------------------------------------
INTERNAL2(void *, KrnCopyMem, INT_KRN_COPY_MEM, const void *, size_t)


//
// -- Function 0x021 -- Release memory back to the kernel heap
//
//    Prototype: Return_t KrnReleaseMem(void *mem);
//    --------------------------------------------------------
INTERNAL1(Return_t, KrnReleaseMem, INT_KRN_RLS_MEM, void *)



//
// -- Function 0x022 -- Return the number of active cores
//    ---------------------------------------------------
INTERNAL0(int, KrnActiveCores, INT_KRN_CORES_ACTIVE)



//
// -- Function 0x023 -- Pause all cores
//    ---------------------------------
INTERNAL0(Addr_t, KrnPauseCores, INT_KRN_PAUSE_CORES)


//
// -- Function 0x024 -- Release all cores
//    -----------------------------------
INTERNAL1(Return_t, KrnReleaseCores, INT_KRN_RELEASE_CORES, Addr_t)



// =====================
// == Timer functions ==
// =====================


//
// -- Function 0x040 -- Get the current timer counter
//
//    Prototype: uint64_t TmrCurrentCount(void);
//    -----------------------------------------------
INTERNAL0(uint64_t, TmrCurrentCount, INT_TMR_CURRENT_COUNT)


//
// -- Function 0x041 -- Perform the timer functions related to a timer tick
//
//    Prototype: uint64_t TmrTick(void);
//    ---------------------------------------------------------------------
INTERNAL0(uint64_t, TmrTick, INT_TMR_TICK)


//
// -- Function 0x042 -- Perform an EOI realted to a interrupt (LAPIC handles)
//
//    Prototype: Return_t TmrEoi(void);
//    -----------------------------------------------------------------------
INTERNAL0(Return_t, TmrEoi, INT_TMR_EOI)



//
// -- Function 0x042 -- Init the AP timers
//
//    Prototype: Return_t TmrApInit(BootInterface_t *)
//    -----------------------------------------------------------------------
INTERNAL1(Return_t, TmrApInit, INT_TMR_REINIT, BootInterface_t *);



// =======================================
// == Physical Memory Manager functions ==
// =======================================


//
// -- Function 0x050 -- Allocate memory from the PMM
//
//    Prototype: Frame_t PmmAllocAligned(bool lowMem, int numBitsAligned, size_t count);
//    ----------------------------------------------------------------------------------
INTERNAL3(Frame_t, PmmAllocAligned, INT_PMM_ALLOC, bool, int, size_t)
inline Frame_t PmmAllocLow(void) { return PmmAllocAligned(true, 12, 1); }
inline Frame_t PmmAlloc(void) { return PmmAllocAligned(false, 12, 1); }


//
// -- Function 0x051 -- Release memory back to the PMM
//
//    Prototype: Return_t PmmReleaseRange(Frame_t frame, size_t count);
//    -----------------------------------------------------------------
INTERNAL2(Return_t, PmmReleaseRange, INT_PMM_RELEASE, Frame_t, size_t)
inline Return_t PmmRelease(Frame_t frame) { return PmmReleaseRange(frame, 1); }


// =========================
// == Scheduler functions ==
// =========================


//
// -- Function 0x060 -- Perform the Scheduler Tick function
//
//    Prototype: Return_t SchTimerTick(uint64_t now);
//    -----------------------------------------------------
INTERNAL1(Return_t, SchTimerTick, INT_SCH_TICK, uint64_t)


//
// -- Function 0x061 -- Create a new Process and add it to the scheduler
//
//    Prototype: Process_t *SchProcessCreate(const char *name, Addr_t startingAddr, Addr_t addrSpace)
//    -----------------------------------------------------------------------------------------------
#define PTY_IDLE 1
#define PTY_LOW  5
#define PTY_NORM 10
#define PTY_HIGH 20
#define PTY_OS   30
INTERNAL4(Process_t *, SchProcessCreate, INT_SCH_CREATE, const char *, Addr_t, Addr_t, int)


//
// -- Function 0x062 -- Make a process Ready to execute
//
//    Prototype: Return_t SchProcessReady(Process_t *proc);
//    -----------------------------------------------------
INTERNAL1(Return_t, SchProcessReady, INT_SCH_READY, Process_t *)


//
// -- Function 0x063 -- Block the current process with the specified status code
//
//    Prototype: Return_t SchProcessBlock(int status, ListHead_t *head);
//    --------------------------------------------------------------------------
#define PRC_INIT        0
#define PRC_RUNNING     1
#define PRC_READY       2
#define PRC_TERM        3
#define PRC_MTXW        4
#define PRC_SEMW        5
#define PRC_DLYW        6
#define PRC_MSGW        7
INTERNAL2(Return_t, SchProcessBlock, INT_SCH_BLOCK, int, ListHead_t *)


//
// -- Function 0x064 -- Unblock the specified Process, placing it on the Ready queue
//
//    Prototype: Return_t SchProcessUnblock(Process_t *proc);
//    ------------------------------------------------------------------------------
INTERNAL1(Return_t, SchProcessUnblock, INT_SCH_UNBLOCK, Process_t *)


//
// -- Function 0x065 -- Put the current process to sleep until a microsecond mark has passed
//
//    Prototype: Return_t SchProcessMicroSleepUntil(uint64_t when);
//    --------------------------------------------------------------------------------------
INTERNAL1(Return_t, SchProcessMicroSleepUntil, INT_SCH_SLEEP_UNTIL, uint64_t)
inline Return_t SchProcessMicroSleep(uint64_t u) { return SchProcessMicroSleepUntil(TmrCurrentCount() + u); }
inline Return_t SchProcessMilliSleep(uint64_t m) { return SchProcessMicroSleepUntil(TmrCurrentCount() + (m * 1000)); }
inline Return_t SchProcessSleep(uint64_t s) { return SchProcessMicroSleepUntil(TmrCurrentCount() + (s * 1000000)); }


//
// -- Function 0x066 -- Ready all processes on a list
//
//    Prototype: Return_t SchReadyList(ListHead_t *head);
//    --------------------------------------------------------------------------------------
INTERNAL1(Return_t, SchReadyList, INT_SCH_READY_LIST, ListHead_t *)


// ========================
// == Debugger functions ==
// ========================


//
// -- Function 0x070 -- Check if the debugger is installed
//
//    Prototype: Return_t DbgInstalled(void);
//    ----------------------------------------------------
INTERNAL0(Return_t, DbgInstalled, INT_DBG_INSTALLED)


//
// -- Function 0x071 -- Register a debugger function
//
//    Prototype: Return_t DbgRegister(DbgModule_t *mod, DbgStates_t *states, DbgTransition_t *transitions);
//    -----------------------------------------------------------------------------------------------------
INTERNAL3(Return_t, _DbgRegister, INT_DBG_REGISTER, DbgModule_t *, DbgState_t *, DbgTransition_t *)
inline Return_t DbgRegister(DbgModule_t *mod, DbgState_t *st, DbgTransition_t *tr)
{
    DbgModule_t *m = (DbgModule_t *)KrnCopyMem(mod, sizeof(DbgModule_t));
    DbgState_t *s = (DbgState_t *)KrnCopyMem(st, sizeof(DbgState_t) * mod->stateCnt);
    DbgTransition_t *t = (DbgTransition_t *)KrnCopyMem(tr, sizeof(DbgTransition_t) * mod->transitionCnt);
    return _DbgRegister(m, s, t);
}


//
// -- Function 0x072 -- Output a string to the debugger
//
//    Prototype: Return_t DbgOutput(const char *str);
//    -------------------------------------------------
INTERNAL1(Return_t, _DbgOutput, INT_DBG_OUTPUT, const char *)
inline Return_t DbgOutput(const char *str)
{
    char *s = (char *)KrnCopyMem((void *)str, kStrLen(str) + 1);
    Return_t rv = _DbgOutput(s);
    KrnReleaseMem(s);
    return rv;
}



//
// -- Function 0x073 -- Prompt for and get generic input
//
//    Prototype: Return_t DbgPromptGeneric(const char *prompt, char *result, size_t size);
//    ------------------------------------------------------------------------------------
INTERNAL3(Return_t, _DbgPromptGeneric, INT_DBG_PROMPT_GENERIC, const char *, char *, size_t)
inline Return_t DbgPromptGeneric(const char *pr, char *rt, size_t s)
{
    const char *p = (const char *)KrnCopyMem((void *)pr, kStrLen(pr) + 1);
    char *r = (char *)KrnCopyMem(rt, s);
    kMemSetB(r, 0, s);

    Return_t rv = _DbgPromptGeneric(p, r, s);

    KrnReleaseMem((void *)p);
    kStrCpy(rt, r);
    KrnReleaseMem(r);

    return rv;
}




// =========================
// == LAPIC/IPI functions ==
// =========================


//
// -- Function 0x080 -- Get the current CPU LAPIC ID
//
//    Prototype: int LapicGetId(void);
//    ----------------------------------------------
INTERNAL0(int, LapicGetId, INT_IPI_CURRENT_CPU)



//
// -- Function 0x081 -- Send the Init IPI to the specified core
//
//    Prototype: int IpiSendInit(int core);
//    ---------------------------------------------------------
INTERNAL1(Return_t, IpiSendInit, INT_IPI_SEND_INIT, int)



//
// -- Function 0x082 -- Send the SIPI IPI to the specified core
//
//    Prototype: int IpiSendSipi(int core, Addr_t vector);
//    ---------------------------------------------------------
INTERNAL2(Return_t, IpiSendSipi, INT_IPI_SEND_SIPI, int, Addr_t)



//
// -- Function 0x083 -- Broadcast an IPI to all CPUs
//
//    Prototype: int IpiSendIpi(int ipi);
//    ---------------------------------------------------------
INTERNAL1(Return_t, IpiSendIpi, INT_IPI_SEND_IPI, int)


#endif


