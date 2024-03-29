//===================================================================================================================
//
//  atomic.h -- These are atomic integer type and functions
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Sometimes a lock is a little heavy-handed for handling a simple integer operation.  An Atomic Integer is a
//  set of operations that are guaranteed to be completed atomically -- or in one cpu instruction.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-14  Initial  v0.0.4   ADCL  Initial version (leveraged)
//
//===================================================================================================================


#pragma once


#include "types.h"


//
// -- this is the atomic integer type
//    -------------------------------
typedef struct AtomicInt_t {
    volatile int64_t counter;
} AtomicInt_t;


//
// -- Initialize in AtomicInt_t at compile time
//    -----------------------------------------
#define ATOMIC_INT_INIT(n)      { (n) }


//
// -- Set an AtomicInt_t to a value, returning the previous value
//    -----------------------------------------------------------
inline int64_t AtomicSet(volatile AtomicInt_t *a, int64_t v) {
    return __atomic_exchange_n(&(a->counter), v, __ATOMIC_SEQ_CST);
}


//
// -- Add a value to an AtomicInt_t, returning the *previous* value
//    -------------------------------------------------------------
inline int64_t AtomicAdd(volatile AtomicInt_t *a, int64_t v) {
    return __atomic_fetch_add(&(a->counter), v, __ATOMIC_SEQ_CST);
}


//
// -- Subtract a value from an AtomicInt_t, returning the previous value
//    ------------------------------------------------------------------
inline int64_t AtomicSub(volatile AtomicInt_t *a, int64_t v) {
    return __atomic_fetch_sub(&(a->counter), v, __ATOMIC_SEQ_CST);
}


//
// -- This function will read an integer atomically (which happens without any special work)
//    --------------------------------------------------------------------------------------
inline int64_t AtomicRead(volatile AtomicInt_t *a) {
    return __atomic_load_n(&(a->counter), __ATOMIC_SEQ_CST);
}


//
// -- This function will increment a value for an atomic interger, returning the previous value
//    -----------------------------------------------------------------------------------------
inline int64_t AtomicInc(volatile AtomicInt_t *a) { return AtomicAdd(a, 1); }


//
// -- This function will decrement a value for an atomic integer, returning the previous value
//    ----------------------------------------------------------------------------------------
inline int64_t AtomicDec(volatile AtomicInt_t *a) { return AtomicSub(a, 1); }


//
// -- This function will atomically add and test if the result is 0
//    -------------------------------------------------------------
inline bool AtomicAddAndNegative(volatile AtomicInt_t *a, int64_t v) {
    return __atomic_add_fetch(&(a->counter), v, __ATOMIC_SEQ_CST) < 0;
}


//
// -- This function will atomically subtract and test if the result is 0
//    ------------------------------------------------------------------
inline bool AtomicSubAndTest0(volatile AtomicInt_t *a, int64_t v) {
    return __atomic_sub_fetch(&(a->counter), v, __ATOMIC_SEQ_CST) == 0;
}


//
// -- This function will atomically increment and test if the result is 0
//    -------------------------------------------------------------------
inline bool AtomicIncAndTest0(volatile AtomicInt_t *a) {
    return __atomic_add_fetch(&(a->counter), 1, __ATOMIC_SEQ_CST) == 0;
}


//
// -- This function will atomically decrement and test if the result is 0
//    -------------------------------------------------------------------
inline bool AtomicDecAndTest0(volatile AtomicInt_t *a) {
    return __atomic_sub_fetch(&(a->counter), 1, __ATOMIC_SEQ_CST) == 0;
}


