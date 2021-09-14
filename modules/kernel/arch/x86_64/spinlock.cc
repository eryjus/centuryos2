//===================================================================================================================
//
//  spinlock.cc -- Code for spinlocks
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-20  Initial   0.0.6   ADCL  Initial version (copied)
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "spinlock.h"


//
// -- Lock a spinlock, busy looping indefinitely until a lock is obtained
//    -------------------------------------------------------------------
int krn_SpinLock(int, Spinlock_t *lock) {
    int exp, des;
    lock->flags = DisableInt();
    while (!__atomic_compare_exchange(&(lock->lock), &exp, &des, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        RestoreInt(lock->flags);        // -- restore interrupts for a short time -- need to allow a process change
        exp = 0;
        des = 1;
        lock->flags = DisableInt();     // -- that's enough; dsiable them again
    }

    return 0;
}


//
// -- Unlock a spinlock, restoring interrupt flag
//    -------------------------------------------
int krn_SpinUnlock(int, Spinlock_t *lock) {
    int l;
    __atomic_load(&(lock->lock), &l, __ATOMIC_SEQ_CST);
    if (l == 0) return -ENOLCK;

    int zero = 0;
    __atomic_store(&(lock->lock), &zero, __ATOMIC_SEQ_CST);
    RestoreInt(lock->flags);
    return 0;
}


//
// -- Determine if a spinlock is locked, lock it if not
//    -------------------------------------------------
int krn_SpinTry(int, Spinlock_t *lock, size_t timeout) {
   int l;
    __atomic_load(&(lock->lock), &l, __ATOMIC_SEQ_CST);
    if (l == 1) return -EBUSY;
    else return 0;
}



