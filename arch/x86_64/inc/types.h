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


//#include <cstdint>
//#include <cstddef>


typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef long int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

typedef unsigned long size_t;
#define NULL __nullptr
#define offsetof(type,member) (Addr_t)(&(((type *)0)->member))


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
// -- The allocated stack size
//    ------------------------
#define STACK_SIZE                      (4096*4)



//
// -- Foundational Types
//    ------------------
typedef uint64_t Frame_t;
typedef uint64_t Addr_t;
typedef uint64_t Pid_t;
typedef uint8_t Byte_t;
typedef uint64_t Bitmap_t;
typedef int64_t Return_t;



//
// -- This is the spinlock structure
//    ------------------------------
typedef struct Spinlock_t {
    volatile int lock;
    Addr_t flags;
} Spinlock_t;



//
// -- This is the common definition of a service routine
//    --------------------------------------------------
typedef struct ServiceRoutine_t {
    Addr_t handler;
    Addr_t cr3;
    Addr_t stack;
    Addr_t runtimeRegs;
    Spinlock_t lock;
} ServiceRoutine_t;



//
// -- Finally pick up some other required files
//    -----------------------------------------
#include "cpu.h"
#include "errno.h"
#include "atomic.h"
#include "lists.h"



//
// -- Stack information
//    -----------------
const Addr_t STACK_LOCATION             = 0xfffff80000000000;


//
// -- Temporary Location Mappings
//    ---------------------------
const Addr_t MMU_STACK_INIT_VADDR       = 0xffffaf8010000000;


#include "constants.h"
#include "debug.h"


extern "C" Addr_t GetAddressSpace(void);


// -- if we are not compiling the loader, include the kernel functions
#ifdef __LOADER__
extern Frame_t earlyFrame;
static inline Frame_t PmmAlloc() { return earlyFrame ++; }
#else
#include "kernel-funcs.h"
#endif


static_assert(sizeof(long) == 8, "requirement: `sizeof(long) == 8` not met");


#endif

