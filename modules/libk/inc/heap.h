//===================================================================================================================
//
//  heap.h -- Kernel Heap structures and functions
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This files contains the structures and definitions needed to manage and control the heap in Century.
//
//  The basis for the design is lifted from Century32 (a 32-bit Hobby OS).
//
//  There are several structures that are used and maintained with the heap management.  The heap structure itself
//  is nothing more than a doubly linked list of free blocks of memory.  This linked list is also ordered based on
//  the size of the free block of memory.  Pointers are setup in the heap structure to point to blocks of certain
//  sizes in an attempt to speed up the allocation and deallocation process.  These pointers are at:
//  * the beginning of the heap (of course)
//  * >= 512 bytes
//  * >= 1K bytes
//  * >= 4K bytes
//  * >= 16K bytes
//
//  When a block of memory is requested, the size if first increased to cover the size of the header and footer as
//  well as adjusted up to the allocation alignment.  So, if 1 byte is requested (unlikely, but great for
//  illustration purposes), the size is increased to HEAP_SMALLEST and then the size of the header (KHeapHdr_size),
//  the size of the footer (KHeapFtr_size), and then aligned to the next 8 byte boundary up.
//
//  Free blocks are maintained in the heap structure as an ordered list by size, from smallest to biggest.  In
//  addition, when the ordered list is searched for the "best fit" (that is the class of algorithm used here), if
//  the adjusted request is >= 16K, then the search starts at the 16K pointer; >= 4K but < 16K, then the search
//  starts at the 4K pointer; >= 1K but < 4K, then the search starts at the 1K pointer; >= 512 bytes but < 1K, then
//  the search starts at the 512 bytes pointer; and, all other searches < 512 bytes are stated at the beginning.
//
//  Note that if there are no memory blocks < 512 bytes, but blocks >= 512 bytes, then the beginning of the ordered
//  list will point to the first block no matter the size.  The rationale for this is simple: a larger block can
//  always be split to fulfill a request.
//
//  On the other hand, if there are no blocks >= 16K bytes is size, then the >= 16K pointer will be NULL.  Again,
//  the rationale is simple: we cannot add up blocks to make a 16K block, so other measures need to be taken (create
//  more heap memory or return failure).
//
//  Finally, the dedicated ordered list array is going to be eliminated in this implementation.  Instead it will be
//  included as part of the header structure.  This change will allow for more than a fixed number of free blocks.
//  This should also simplify the implementation as well.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-May-11  Initial  v0.0.9a  ADCL  Initial version -- Leveraged from Century-OS
//
//===================================================================================================================


#pragma once


#include "types.h"


//#define DEBUG_HEAP

extern "C" {
    //
    // -- Allocate  memory from the heap
    //    ------------------------------
    void *HeapAlloc(size_t size, bool align);


    //
    // -- Free a block of memory
    //    ----------------------
    void HeapFree(void *mem);


    //
    // -- Initialize the Heap
    //    -------------------
    void HeapInit(void);


    //
    // -- A quick macro to make coding easier and more readable
    //    -----------------------------------------------------
    #define NEW(tp)         ({tp* rv = (tp *)HeapAlloc(sizeof(tp), false); rv;})
    #define FREE(ptr)       HeapFree(ptr)
}

