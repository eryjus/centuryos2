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


#pragma once

#include "types.h"



//
// -- these are the internal functions provided by the kernel
//    -------------------------------------------------------
enum {
    INT_GET_HANDLER         = 0,
    INT_SET_HANDLER         = 1,
    INT_GET_SERVICE         = 2,
    INT_SET_SERVICE         = 3,
    INT_GET_INTERRUPT       = 4,
    INT_SET_INTERRUPT       = 5,
    INT_MMU_MAP             = 6,
    INT_MMU_UNMAP           = 7,
    INT_MMU_DUMP_TABLES     = 8,
    INT_MMU_IS_MAPPED       = 9,
    INT_PMM_ALLOC           = 10,
    INT_PMM_RELEASE         = 11,
    INT_TMR_CURRENT         = 13,
    INT_SPIN_LOCK           = 16,
    INT_SPIN_TRY            = 17,
    INT_SPIN_UNLOCK         = 18,
    INT_PRINTF              = 20,
    INT_SCH_TICK            = 25,
    INT_SCH_BLOCK           = 26,
    INT_SCH_READY           = 27,
    INT_SCH_UNBLOCK         = 28,
    INT_SCH_SLEEP_UNTIL     = 29,
    INT_SCH_CREATE          = 30,
};



//
// -- These are the interface functions
//    ---------------------------------
extern "C" {
    int InternalDispatch0(int func);
    int InternalDispatch1(int func, Addr_t p1);
    int InternalDispatch2(int func, Addr_t p1, Addr_t p2);
    int InternalDispatch3(int func, Addr_t p1, Addr_t p2, Addr_t p3);
    int InternalDispatch4(int func, Addr_t p1, Addr_t p2, Addr_t p3, Addr_t p4);
    int InternalDispatch5(int func, Addr_t p1, Addr_t p2, Addr_t p3, Addr_t p4, Addr_t p5);
    int InternalVariable(int func, Addr_t p1, ...);
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
    void kStrCpy(char *dest, const char *src);
    size_t kStrLen(const char *str);
}



//
// -- all the function prototypes
//    ---------------------------
extern "C" {
    // -- Function 0
    int GetInternalHandler(int number);

    // -- Function 1
    int SetInternalHandler(int number, Addr_t handlerAddr, Addr_t cr3);

    // -- Function 2
    int GetInternalService(int number);

    // -- Function 3
    int SetInternalService(int number, Addr_t serviceAddr, Addr_t cr3);

    // -- Function 4
    int GetInterruptHandler(int number);

    // -- Function 5
    int SetInterruptHandler(int number, Addr_t selector, Addr_t interruptAddr, int ist, int dpl);

    // -- Function 6
#define PG_NONE     (0)
#define PG_WRT      (1<<0)
#define PG_KRN      (1<<1)
#define PG_DEV      (1<<15)
    int MmuMapPage(Addr_t addr, Frame_t frame, int flags);

    // -- Function 7
    int MmuUnmapPage(Addr_t addr);

    // -- Function 8
    int MmuDumpTables(Addr_t addr);

    // -- Function 9
    bool MmuIsMapped(Addr_t addr);

    // -- Function 10
    Frame_t PmmAllocAligned(bool lowMem, int numBitsAligned, size_t count);
    Frame_t PmmAllocLow(void);
    Frame_t PmmAlloc(void);

    // -- Function 11
    int PmmReleaseRange(Frame_t frame, size_t count);
    int PmmRelease(Frame_t frame);

    // -- Function 13
    uint64_t TmrCurrentCount(void);

    // -- Function 16
    int SpinLock(Spinlock_t *lock);

    // -- Function 17
    int SpinTry(Spinlock_t *lock, size_t timeout);

    // -- Function 18
    int SpinUnlock(Spinlock_t *lock);

    // -- Function 20
    int KernelPrintf(const char *fmt, ...);

    // -- Function 25
    int SchTimerTick(uint64_t now);

    // -- Function 26
#define PRC_INIT        0
#define PRC_RUNNING     1
#define PRC_READY       2
#define PRC_TERM        3
#define PRC_MTXW        4
#define PRC_SEMW        5
#define PRC_DLYW        6
#define PRC_MSGW        7
    int SchProcessBlock(int status);

    // -- Function 27
    int SchProcessReady(Process_t *proc);

    // -- Function 28
    int SchProcessUnblock(Process_t *proc);

    // -- Function 29
    int SchProcessMicroSleepUntil(uint64_t when);
    inline int ProcessMicroSleep(uint64_t micros) { return SchProcessMicroSleepUntil(TmrCurrentCount() + micros); }
    inline int ProcessMilliSleep(uint64_t ms) { return SchProcessMicroSleepUntil(TmrCurrentCount() + (ms * 1000)); }
    inline int ProcessSleep(uint64_t s) { return SchProcessMicroSleepUntil(TmrCurrentCount() + (s * 1000000)); }

    // -- Function 30
    int SchProcessCreate(const char *name, void (*startingAddr)(void));
}

