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
    // -- Internal Table Maintenance operations
    INT_GET_INTERNAL        = 0x000,
    INT_SET_INTERNAL        = 0x001,
    INT_GET_VECTOR          = 0x002,
    INT_SET_VECTOR          = 0x003,
    INT_GET_SERVICE         = 0x004,
    INT_SET_SERVICE         = 0x005,

    // -- Output functions
    INT_PRINTF              = 0x008,

    // -- Spinlock functions
    INT_KRN_SPIN_LOCK       = 0x010,
    INT_KRN_SPIN_TRY        = 0x011,
    INT_KRN_SPIN_UNLOCK     = 0x012,

    // -- MMU Functions
    INT_KRN_MMU_MAP         = 0x018,
    INT_KRN_MMU_UNMAP       = 0x019,
    INT_KRN_MMU_IS_MAPPED   = 0x01a,
    INT_KRN_MMU_DUMP        = 0x01b,

    // -- Kernel Utility Functions
    INT_KRN_COPY_MEM        = 0x020,
    INT_KRN_RLS_MEM         = 0x021,

    // -- Timer Module Functions
    INT_TMR_CURRENT_COUNT   = 0x040,

    // -- PMM Module Functions
    INT_PMM_ALLOC           = 0x050,
    INT_PMM_RELEASE         = 0x051,

    // -- Scheduler Module Functions
    INT_SCH_TICK            = 0x060,
    INT_SCH_CREATE          = 0x061,
    INT_SCH_READY           = 0x062,
    INT_SCH_BLOCK           = 0x063,
    INT_SCH_UNBLOCK         = 0x064,
    INT_SCH_SLEEP_UNTIL     = 0x065,
};



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
    size_t kStrLen(const char *str);
    Addr_t GetAddressSpace(void);
}



//
// -- all the function prototypes
//    ---------------------------
extern "C" {
    // -- Function 0
    Return_t GetInternalHandler(int number);

    // -- Function 1
    Return_t SetInternalHandler(int number, Addr_t handlerAddr, Addr_t cr3, Addr_t stack);

    // -- Function 2
    Return_t GetVectorHandler(int number);

    // -- Function 3
    Return_t SetVectorHandler(int number, Addr_t vectorAddr, Addr_t cr3, Addr_t stack);

    // -- Function 4
    Return_t GetServiceHandler(int number);

    // -- Function 5
    Return_t SetServiceHandler(int number, Addr_t serviceAddr, Addr_t cr3, Addr_t stack);


    // -- Function 8
    Return_t KernelPrintf(const char *fmt, ...);


    // -- Function 16
    Return_t SpinLock(Spinlock_t *lock);

    // -- Function 17
    Return_t SpinTry(Spinlock_t *lock, size_t timeout);

    // -- Function 18
    Return_t SpinUnlock(Spinlock_t *lock);


    // -- Function 24
#define PG_NONE     (0)
#define PG_WRT      (1<<0)
#define PG_KRN      (1<<1)
#define PG_DEV      (1<<15)
    Return_t MmuMapPage(Addr_t addr, Frame_t frame, int flags);

    // -- Function 25
    Return_t MmuUnmapPage(Addr_t addr);

    // -- Function 26
    bool MmuIsMapped(Addr_t addr);

    // -- Function 27
    Return_t MmuDump(Addr_t addr);


    // -- Function 32
    void *KrnCopyMem(void *mem, size_t size);

    // -- Function 33
    Return_t KrnReleaseMem(void *mem);





    // -- Function 10
    Frame_t PmmAllocAligned(bool lowMem, int numBitsAligned, size_t count);
    Frame_t PmmAllocLow(void);
    Frame_t PmmAlloc(void);

    // -- Function 11
    Return_t PmmReleaseRange(Frame_t frame, size_t count);
    Return_t PmmRelease(Frame_t frame);

    // -- Function 13
    uint64_t TmrCurrentCount(void);

    // -- Function 25
    Return_t SchTimerTick(uint64_t now);

    // -- Function 26
#define PRC_INIT        0
#define PRC_RUNNING     1
#define PRC_READY       2
#define PRC_TERM        3
#define PRC_MTXW        4
#define PRC_SEMW        5
#define PRC_DLYW        6
#define PRC_MSGW        7
    Return_t SchProcessBlock(int status);

    // -- Function 27
    Return_t SchProcessReady(Process_t *proc);

    // -- Function 28
    Return_t SchProcessUnblock(Process_t *proc);

    // -- Function 29
    Return_t SchProcessMicroSleepUntil(uint64_t when);
    inline Return_t ProcessMicroSleep(uint64_t micros) { return SchProcessMicroSleepUntil(TmrCurrentCount() + micros); }
    inline Return_t ProcessMilliSleep(uint64_t ms) { return SchProcessMicroSleepUntil(TmrCurrentCount() + (ms * 1000)); }
    inline Return_t ProcessSleep(uint64_t s) { return SchProcessMicroSleepUntil(TmrCurrentCount() + (s * 1000000)); }

    // -- Function 30
    Return_t SchProcessCreate(const char *name, void (*startingAddr)(void), Addr_t addrSpace);
}

