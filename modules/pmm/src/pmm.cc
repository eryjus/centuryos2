//===================================================================================================================
//
//  pmm.cc -- Functions to handle the PMM management
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-12  Initial  v0.0.4   ADCL  Initial version
//
//===================================================================================================================


// #define USE_SERIAL

#include "types.h"
#include "boot-interface.h"
#include "kernel-funcs.h"
#include "serial.h"


//
// -- Some function prototypes
//    ------------------------
extern "C" {
    int PmmInitEarly(BootInterface_t *loaderInterface);
    Frame_t PmmAllocateAligned(bool low, int bitsAligned, size_t count);
    int PmmReleaseFrame(Frame_t frame, size_t count);
}



//
// -- Addresses for this PMM
//    ----------------------
const Addr_t tempMap = 0xffffff0000000000;
//const Addr_t lowStack = 0xffffff0000001000;
//const Addr_t normStack = 0xffffff0000002000;
const Addr_t scrubStack = 0xffffff0000003000;


//
// -- This is the new PMM frame information structure -- contains info about this block of frames
//    -------------------------------------------------------------------------------------------
typedef struct PmmFrameInfo_t {
    Frame_t frame;
    size_t count;
    Frame_t prev;
    Frame_t next;
} PmmFrameInfo_t;



//
// -- This is the new PMM itself
//    --------------------------
typedef struct Pmm_t {
    AtomicInt_t framesAvail;                // -- this is the total number of frames available in the 3 stacks

    Spinlock_t lowLock;                     // -- This lock protects lowStack
    PmmFrameInfo_t *lowStack;

    Spinlock_t normLock;                    // -- This lock protects normStack
    PmmFrameInfo_t *normStack;

    Spinlock_t scrubLock;                   // -- This lock protects scrubStack
    PmmFrameInfo_t *scrubStack;
} Pmm_t;



//
// -- Finally, the actual structure in memory
//    ---------------------------------------
Pmm_t pmm = { { 0 } };



//
// -- Some things that we need to scrape from the interface structure
//    ---------------------------------------------------------------
Frame_t nextFrame;


//
// -- Complete the initialization required for the PMM
//    ------------------------------------------------
int PmmInitEarly(BootInterface_t *loaderInterface)
{
    MmuMapPage(tempMap, 0, false);              // -- this needs to be mapped to allocate new tables
    MmuUnmapPage(tempMap);                      // -- unmap immediately -- tables created

    for (int i = 0; i < MAX_MEM; i ++) {
        Addr_t start = loaderInterface->memBlocks[i].start;
        Addr_t end = loaderInterface->memBlocks[i].end;

        if (start == 0 && end == 0) continue;
        if (start < loaderInterface->nextEarlyFrame << 12 && end > loaderInterface->nextEarlyFrame << 12) {
            start = loaderInterface->nextEarlyFrame << 12;
        }

        Addr_t size = (end - start) >> 12;

        MmuMapPage(tempMap, start >> 12, true);

        PmmFrameInfo_t *info = (PmmFrameInfo_t *)tempMap;
        info->count = (end - start) >> 12;
        info->frame = size;
        info->next = 0;
        info->prev = 0;

        AtomicAdd(&pmm.framesAvail, size);

        // -- Push onto the stack
        if (pmm.scrubStack) {
            pmm.scrubStack->prev = info->frame;
            info->next = pmm.scrubStack->frame;

            MmuUnmapPage((Addr_t)pmm.scrubStack);
            MmuMapPage((Addr_t)pmm.scrubStack, info->frame, true);
        } else {
            MmuMapPage((Addr_t)scrubStack, info->frame, true);
            pmm.scrubStack = (PmmFrameInfo_t *)scrubStack;
        }

        MmuUnmapPage(tempMap);
    }

    nextFrame = loaderInterface->nextEarlyFrame;
    SetInternalHandler(INT_PMM_ALLOC, (InternalHandler_t)PmmAllocateAligned, GetPageTables());

    return 0;   // will be loaded
}


//
// -- Allocate a frame
//    ----------------
Frame_t PmmAllocateAligned(bool low, int bitsAligned, size_t count)
{
    return -ENOMEM;
}


//
// -- Release a frame
//    ---------------
int PmmReleaseFrame(Frame_t frame, size_t count)
{
    return 0;
}

