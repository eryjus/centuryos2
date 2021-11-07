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
// -- Function prototypes
//    -------------------
extern "C" {
    Return_t krn_SpinLock(Spinlock_t *lock);
    Return_t krn_SpinUnlock(Spinlock_t *lock);
    Return_t krn_SpinTry(Spinlock_t *lock, size_t timeout);
}



#endif

