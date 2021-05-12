//====================================================================================================================
//
//  types.h -- Foundational types for CenturyOS
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-14  Initial  v0.0.2   ADCL  Initial version
//
//===================================================================================================================


#pragma once
#ifndef __TYPES_H__
#define __TYPES_H__


#include <cstdint>
#include <cstddef>


//
// -- Some constants for the compile
//    ------------------------------

// -- !!!DANGER!!! -- has static coding impications; check GDT before modifying
#define MAX_CPU 4


//
// -- This is the natural byte alignment for this architecture
//    --------------------------------------------------------
#define BYTE_ALIGNMENT      8


//
// -- Page Size
//    ---------
#define PAGE_SIZE           4096


//
// -- Foundational Types
//    ------------------
typedef uint64_t Frame_t;
typedef uint64_t Addr_t;
typedef uint64_t Pid_t;
typedef uint8_t Byte_t;


//
// -- This is the type definition of a handler function
//    -------------------------------------------------
typedef void (*IdtHandlerFunc_t)(Addr_t *);


//
// -- this is the internal handler function address, used for the table
//    (each function has its own parameter list)
//    -----------------------------------------------------------------
typedef Addr_t InternalHandler_t;
typedef Addr_t ServiceHandler_t;


//
// -- This is the spinlock structure
//    ------------------------------
typedef struct Spinlock_t {
    volatile int lock;
    Addr_t flags;
} Spinlock_t;




//
// -- Finally pick up some other required files
//    -----------------------------------------
#include "cpu.h"
#include "errno.h"
#include "atomic.h"
#include "lists.h"


#endif

