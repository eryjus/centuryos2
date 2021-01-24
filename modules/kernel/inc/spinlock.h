//===================================================================================================================
//
//  spinlock.h -- Structures for spinlock management
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-20  Initial   0.0.3   ADCL  Initial version
//
//===================================================================================================================


#pragma once
#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__


#include "types.h"
#include "cpu.h"


//
// -- This inline function will lock a spinlock, busy looping indefinitely until a lock is obtained
//    ---------------------------------------------------------------------------------------------
inline void SpinLock(Spinlock_t *lock) {
    int exp, des;
    lock->flags = DisableInt();
    do {
        RestoreInt(lock->flags);        // restore interrupts for a short time -- need to allow a process change
        exp = 0;
        des = 1;
        lock->flags = DisableInt();     // that's enough; enable them again
    } while (!__atomic_compare_exchange(&(lock->lock), &exp, &des, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
}


//
// -- This inline function will unlock a spinlock, clearing the lock holder
//    ---------------------------------------------------------------------
inline void SpinUnlock(Spinlock_t *lock) {
    int zero = 0;
    __atomic_store(&(lock->lock), &zero, __ATOMIC_SEQ_CST);
    RestoreInt(lock->flags);
}


//
// -- This inline function will determine if a spinlock is locked
//    -----------------------------------------------------------
inline int SpinlockIsLocked(Spinlock_t *lock) {
    int l;
    __atomic_load(&(lock->lock), &l, __ATOMIC_SEQ_CST);
    if (l == 1) return -EBUSY;
    else return 0;
}


#endif

