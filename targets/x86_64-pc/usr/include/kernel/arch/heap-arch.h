//===================================================================================================================
//
//  heap.h -- Kernel Heap structures and functions
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-May-11  Initial  v0.0.9a  ADCL  Initial version -- Leveraged from Century-OS
//
//===================================================================================================================


#pragma once


//
// -- Define some quick macros to help with managing the heap
//    -------------------------------------------------------
#define HEAP_CHECK_MASK         0xfffffffffffffffe
#define HEAP_SMALLEST           128
#define HEAP_MAGIC              ((Addr_t)0xBAD2DB07EBA6BADC)
#define HEAP_CHECK(x)           (((x) & HEAP_CHECK_MASK) == HEAP_MAGIC)
#define MIN_HOLE_SIZE           (sizeof(KHeapHeader_t) + sizeof(KHeapHeader_t) + HEAP_SMALLEST)

#define HEAP_MIN_SIZE           0x00040000
#define HEAP_SIZE_INCR          HEAP_MIN_SIZE
#define ORDERED_LIST_STATIC     (4096)


//
// -- arch-specific magic-hole
//    ------------------------
typedef struct MagicHole_t {
    Addr_t isHole : 1;               // == 1 if this is a hole (not used)
    Addr_t magic : 63;               // magic number
} MagicHole_t;


