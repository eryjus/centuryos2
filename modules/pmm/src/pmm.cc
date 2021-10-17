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


#include "types.h"
#include "boot-interface.h"
#include "kernel-funcs.h"


//#define PMM_DEBUG


//
// -- Some function prototypes
//    ------------------------
extern "C" {
    Return_t PmmInitEarly(BootInterface_t *loaderInterface);
    Frame_t pmm_PmmAllocateAligned(int, bool low, int bitsAligned, size_t count);
    Return_t pmm_PmmReleaseFrame(int, Frame_t frame, size_t count);
    Addr_t GetCr3(void);
    void pmm_LateInit(void);
}



//
// -- Addresses for this PMM
//    ----------------------
const Addr_t tempMap    = 0xffffff0000000000;
const Addr_t lowStack   = 0xffffff0000001000;
const Addr_t normStack  = 0xffffff0000002000;
const Addr_t scrubStack = 0xffffff0000003000;
const Addr_t tempInsert = 0xffffff0000004000;
const Addr_t clearAddr  = 0xffffff0000005000;


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

    Spinlock_t searchLock;                  // -- This lock protects search
    PmmFrameInfo_t *search;

    // -- some additional locks for mapped address usage
    Spinlock_t insertLock;
    Spinlock_t clearLock;
} Pmm_t;



//
// -- Finally, the actual structure in memory
//    ---------------------------------------
Pmm_t pmm = { { 0 } };



//
// -- For debugging, dump the PMM internal structure
//    ----------------------------------------------
void DumpPmm(void)
{
    KernelPrintf("\n");
    KernelPrintf("==========================================\n");
    KernelPrintf("\n");
    KernelPrintf("Dumping PMM Structure:\n");
    KernelPrintf("  Number of frames available: %ld\n", pmm.framesAvail);
    KernelPrintf("\n");
    KernelPrintf("  Low Lock State: %s\n", pmm.lowLock.lock?"locked":"unlocked");
    KernelPrintf("  Low Stack Address: %p\n", pmm.lowStack);

    if (pmm.lowStack) {
        KernelPrintf("    Low Stack TOS frame: %p\n", pmm.lowStack->frame);
        KernelPrintf("    Low Stack TOS count: %d\n", pmm.lowStack->count);
    }

    KernelPrintf("\n");
    KernelPrintf("  Normal Lock State: %s\n", pmm.normLock.lock?"locked":"unlocked");
    KernelPrintf("  Normal Stack Address: %p\n", pmm.normStack);

    if (pmm.normStack) {
        KernelPrintf("    Normal Stack TOS frame: %p\n", pmm.normStack->frame);
        KernelPrintf("    Normal Stack TOS count: %d\n", pmm.normStack->count);
    }

    KernelPrintf("\n");
    KernelPrintf("  Scrub Lock State: %s\n", pmm.scrubLock.lock?"locked":"unlocked");
    KernelPrintf("  Scrub Stack Address: %p\n", pmm.scrubStack);

    if (pmm.scrubStack) {
        KernelPrintf("    Scrub Stack TOS frame: %p\n", pmm.scrubStack->frame);
        KernelPrintf("    Scrub Stack TOS count: %d\n", pmm.scrubStack->count);
    }

    KernelPrintf("\n");
    KernelPrintf("  Search Lock State: %s\n", pmm.searchLock.lock?"locked":"unlocked");
    KernelPrintf("  Search Stack Address: %p\n", pmm.search);

    if (pmm.search) {
        KernelPrintf("    Search Stack TOS frame: %p\n", pmm.search->frame);
        KernelPrintf("    Search Stack TOS count: %d\n", pmm.search->count);
    }

    KernelPrintf("\n");
    KernelPrintf("==========================================\n");
    KernelPrintf("\n");
}


//
// -- Push frames on a stack (must hold the stack lock to call)
//    ---------------------------------------------------------
static void PushStack(PmmFrameInfo_t **stack, Frame_t frame, size_t count)
{
    SpinLock(&pmm.insertLock);
    MmuMapPage(tempInsert, frame, PG_WRT);
    volatile PmmFrameInfo_t *wrk = (PmmFrameInfo_t *)tempInsert;

    wrk->frame = frame;
    wrk->count = count;
    wrk->prev = 0;

    if (*stack) {
        wrk->next = (*stack)->frame;
        (*stack)->prev = frame;

        MmuUnmapPage((Addr_t)(*stack));
    } else {
        wrk->next = 0;

        if (stack == &pmm.lowStack) *stack = (PmmFrameInfo_t *)lowStack;
        else if (stack == &pmm.normStack) *stack = (PmmFrameInfo_t *)normStack;
        else if (stack == &pmm.scrubStack) *stack = (PmmFrameInfo_t *)scrubStack;
        else KernelPrintf("PushStack(): Unable to determine on which stack to push\n");
    }

    MmuUnmapPage(tempInsert);
    SpinUnlock(&pmm.insertLock);

    // -- finally, push the new node
    MmuMapPage((Addr_t)(*stack), frame, PG_WRT);
}



//
// -- Pop the last frame off the stack (must hold the stack lock to call)
//    -------------------------------------------------------------------
static void PopStack(PmmFrameInfo_t **stack)
{
    if (!(*stack)) return;

    Frame_t nx = (*stack)->next;

    // -- clear out the data elements! -- prev is already 0
    (*stack)->count = 0;
    (*stack)->frame = 0;
    (*stack)->next = 0;

    MmuUnmapPage((Addr_t)(*stack));

    if (nx) {
        MmuMapPage((Addr_t)(*stack), nx, PG_WRT);
        (*stack)->prev = 0;
    }
}



//
// -- Scrub a frame, clearing its contents
//    ------------------------------------
static void PmmScrubFrame(Frame_t frame)
{
    SpinLock(&pmm.clearLock);

#ifdef PMM_DEBUG
    KernelPrintf("!!!!! PmmScrubFrame() preparing to map a page\n");
#endif
    MmuMapPage(clearAddr, frame, PG_WRT);
#ifdef PMM_DEBUG
    KernelPrintf("!!!!! PmmScrubFrame() page mapped\n");
    MmuDump(clearAddr);
#endif

    uint64_t *wrk = (uint64_t *)clearAddr;

    for (int i = 0; i < PAGE_SIZE / sizeof(uint64_t); i ++) wrk[i] = 0;

    MmuUnmapPage(clearAddr);

    SpinUnlock(&pmm.clearLock);
}



//
// -- Scrub a block of frames
//    -----------------------
static void PmmScrubBlock(Frame_t start, size_t count)
{
    for (size_t i = start; i < start + count; i ++) PmmScrubFrame(i);
}



//
// -- Remove a Frame from a block (the stack lock must be held to call)
//    -----------------------------------------------------------------
static Frame_t PmmDoRemoveFrame(PmmFrameInfo_t **stack, bool scrub)
{
    Frame_t rv = 0;         // assume we will not find anything

#ifdef PMM_DEBUG
    KernelPrintf("Removing a single frame; some interesting facts:\n");
    KernelPrintf(".. stack = %p\n", stack);
    KernelPrintf(".. *stack = %p\n", stack?*stack:0);
#endif

    if ((Addr_t)(*stack)) {
        rv = (*stack)->frame + (*stack)->count - 1;
        (*stack)->count --;
        AtomicDec(&pmm.framesAvail);

        if ((*stack)->count == 0) {
            PopStack(stack);
        }
    } else {
        return 0;
    }

#ifdef PMM_DEBUG
    KernelPrintf(".. Found the frame to use: %p\n", rv);
#endif


    // -- scrub the frame if requested
    if (scrub) PmmScrubFrame(rv);

#ifdef PMM_DEBUG
    KernelPrintf(".. all good!  Returning frame %p\n", rv);
#endif

    return rv;
}



//
// -- Complete the initialization required for the PMM
//    ------------------------------------------------
Return_t PmmInitEarly(BootInterface_t *loaderInterface)
{
    ProcessInitTable();
    KernelPrintf("PmmInitEarly(): Starting initialization\n");

    for (int i = 0; i < MAX_MEM; i ++) {
        Frame_t start = loaderInterface->memBlocks[i].start >> 12;
        Frame_t end = loaderInterface->memBlocks[i].end >> 12;

        if (start == 0 && end == 0) continue;
        if (start < loaderInterface->nextEarlyFrame && end <= loaderInterface->nextEarlyFrame) continue;
        if (start < loaderInterface->nextEarlyFrame) start = loaderInterface->nextEarlyFrame + 0x100;

        Addr_t size = end - start;
        MmuMapPage(tempMap, start, PG_WRT);

        PmmFrameInfo_t *info = (PmmFrameInfo_t *)tempMap;
        info->count = size;
        info->frame = start;
        info->next = 0;
        info->prev = 0;

        AtomicAdd(&pmm.framesAvail, size);

        // -- Push onto the stack
        if (pmm.scrubStack) {
            pmm.scrubStack->prev = info->frame;
            info->next = pmm.scrubStack->frame;

            MmuUnmapPage((Addr_t)pmm.scrubStack);
            MmuMapPage((Addr_t)pmm.scrubStack, info->frame, PG_WRT);
        } else {
            MmuMapPage((Addr_t)scrubStack, info->frame, PG_WRT);
            pmm.scrubStack = (PmmFrameInfo_t *)scrubStack;
        }

        MmuUnmapPage(tempMap);
    }

    SetInternalHandler(INT_PMM_ALLOC, (Addr_t)pmm_PmmAllocateAligned, GetAddressSpace(), 0);

    KernelPrintf("PmmInitEarly(): Initialization complete\n");
//    DumpPmm();

    return 0;   // will be loaded
}



//
// -- Allocate a single frame (normal or low)
//    ---------------------------------------
static Frame_t PmmAllocate(bool low)
{
    Frame_t rv = 0;         // assume we will not find anything


    //
    // -- check the normal stack for a frame to allocate
    //    ----------------------------------------------
    if (!low) {
#ifdef PMM_DEBUG
        KernelPrintf("Allocating a single frame from the normal stack\n");
#endif
        SpinLock(&pmm.normLock);
        rv = PmmDoRemoveFrame(&pmm.normStack, false);
        SpinUnlock(&pmm.normLock);
#ifdef PMM_DEBUG
        KernelPrintf("Frame Allocated: %p\n", rv);
#endif
    }
    if (rv != 0) return rv;


    //
    // -- check the scrub queue for a frame to allocate
    //    ---------------------------------------------
    if (!low) {
#ifdef PMM_DEBUG
        KernelPrintf("Allocating a single frame from the scrub stack\n");
#endif
        SpinLock(&pmm.scrubLock);
        rv = PmmDoRemoveFrame(&pmm.scrubStack, true);       // -- scrub the frame when it is removed
        SpinUnlock(&pmm.scrubLock);
#ifdef PMM_DEBUG
        KernelPrintf("Frame Allocated: %p\n", rv);
#endif
    }
    if (rv != 0) return rv;


    //
    // -- check the low stack for a frame to allocate
    //    -------------------------------------------
#ifdef PMM_DEBUG
    KernelPrintf("Allocating a single frame from the low stack\n");
#endif
    SpinLock(&pmm.lowLock);
    rv = PmmDoRemoveFrame(&pmm.lowStack, false);
    SpinUnlock(&pmm.lowLock);
#ifdef PMM_DEBUG
    KernelPrintf("Frame Allocated: %p\n", rv);
#endif
    if (rv != 0) return rv;

    return -ENOMEM;
}



//
// -- Split the block as needed to pull out the proper alignment and size of frames
//    -----------------------------------------------------------------------------
static Frame_t PmmSplitBlock(PmmFrameInfo_t **stack, Frame_t frame, size_t blockSize, Frame_t atFrame, size_t count)
{
    if (frame < atFrame) {
        // -- Create a new block with the leading frames
        PushStack(stack, frame, atFrame - blockSize);

        // -- adjust the existing block
        frame = atFrame;
        blockSize -= (atFrame - blockSize);
    }


    // -- check for frames to remove at the end of this block; or free the block since it is not needed
    if (blockSize > count) {
        // -- adjust this block to remove what we want
        frame += count;
        blockSize -= count;

        // -- finally push this block back onto the stack
        PushStack(stack, frame, blockSize);
    }


    // -- what is left at this point is `count` frames at `atFrame`; return this value
    return atFrame;
}



//
// -- This function is the working to find a frame that is properly aligned and allocate multiple contiguous frames
//    -------------------------------------------------------------------------------------------------------------
Frame_t PmmDoAllocAlignedFrames(PmmFrameInfo_t **pStack, const size_t count, const size_t bitAlignment)
{
//    KernelPrintf("Allocating aligned frame(s)\n");

    //
    // -- start by determining the bits we cannot have enabled when we evaluate a frame
    //    -----------------------------------------------------------------------------
    Frame_t frameBits = ~(((Frame_t)-1) << (bitAlignment<12?0:bitAlignment-12));
    Frame_t rv = 0;

    if (!MmuIsMapped((Addr_t)*pStack)) return -ENOMEM;


    // -- Interrupts are already disabled before we get here
//    KernelPrintf(".. Attempting the search lock\n");

    SpinLock(&pmm.searchLock); {
//        KernelPrintf(".. Lock obtained\n");

//        KernelPrintf("Some interesting facts before MmuMapPage:\n");
//        KernelPrintf(".. pmm.search is %p\n", pmm.search);
//        KernelPrintf(".. pStack is %p (%s)\n", pStack, MmuIsMapped((Addr_t)pStack)?"mapped":"not mapped");
//        KernelPrintf(".. (*pStack) is %p\n", *pStack);
//        KernelPrintf(".. (*pStack)->frame is %p\n", pStack?(*pStack)->frame:0);
        MmuMapPage((Addr_t)pmm.search, (*pStack)->frame, PG_WRT);

//        KernelPrintf(".. Page mapped\n");

        while(true) {
            Frame_t end = pmm.search->frame + pmm.search->count - 1;
            Frame_t next;

            // -- here we determine if the block is big enough
            if (((pmm.search->frame + frameBits) & ~frameBits) + count - 1 <= end) {
                Frame_t p = pmm.search->prev;
                Frame_t n = pmm.search->next;
                Frame_t f = pmm.search->frame;
                size_t sz = pmm.search->count;

                MmuUnmapPage((Addr_t)pmm.search);

                if (n) {
                    MmuMapPage((Addr_t)pmm.search, n, PG_WRT);
                    pmm.search->prev = p;
                    MmuUnmapPage((Addr_t)pmm.search);
                }

                if (p) {
                    MmuMapPage((Addr_t)pmm.search, p, PG_WRT);
                    pmm.search->next = n;
                    MmuUnmapPage((Addr_t)pmm.search);
                }

                rv = PmmSplitBlock(pStack, f, sz, (f + frameBits) & ~frameBits, count);
                goto exit;
            }

            // -- move to the next node
            next = pmm.search->next;
            MmuUnmapPage((Addr_t)pmm.search);

            // -- here we check if we made it to the end of the stack
            if (next) MmuMapPage((Addr_t)pmm.search, next, PG_WRT);
            else goto exit;
        }

exit:

        SpinUnlock(&pmm.searchLock);
    }

//    KernelPrintf("Aligned PMM Allocation is finally returning frame %x\n", rv);

    return rv;
}



//
// -- Allocate aligned frame(s)
//    -------------------------
Frame_t pmm_PmmAllocateAligned(int, bool low, int bitsAligned, size_t count)
{
#ifdef PMM_DEBUG
    KernelPrintf("Internal Function to allocate aligned frames (pmm_PmmAllocateAligned)\n");
    KernelPrintf(".. low = %s; alignment bits = %d; count = %d\n", low?"true":"false", bitsAligned, count);
#endif

    if (bitsAligned < 12) bitsAligned = 12;
    if (count == 1 && bitsAligned == 12) {
        Frame_t rv = PmmAllocate(low);
#ifdef PMM_DEBUG
        KernelPrintf(".. Returning back to the user frame %p\n", rv);
#endif

        return rv;
    }

    // -- TODO: add some detail here to allocate aligned/multiple frames
#ifdef PMM_DEBUG
    KernelPrintf(".. More than a trivial allocation\n");
#endif
    Addr_t flags = DisableInt();
    Frame_t rv = 0;

    if (low) {
        SpinLock(&pmm.lowLock);
        rv = PmmDoAllocAlignedFrames(&pmm.lowStack, count, bitsAligned);
        SpinUnlock(&pmm.lowLock);
    } else {
        SpinLock(&pmm.normLock);
        rv = PmmDoAllocAlignedFrames(&pmm.normStack, count, bitsAligned);
        SpinUnlock(&pmm.normLock);
    }

    RestoreInt(flags);

    return rv;
}


//
// -- Release a frame
//    ---------------
Return_t pmm_PmmReleaseFrame(int, Frame_t frame, size_t count)
{
    SpinLock(&pmm.scrubLock);
    PushStack(&pmm.scrubStack, frame, count);
    AtomicAdd(&pmm.framesAvail, count);
    SpinUnlock(&pmm.scrubLock);

    DumpPmm();

//    MessageQueueSend(butlerMsgq, BUTLER_CLEAN_PMM, 0, 0);
    return 0;
}



//
// -- This is the process that will keep the scrub stack clean
//    --------------------------------------------------------
void PmmCleanProcess(void)
{
    KernelPrintf("Starting the PMM Cleaner\n");
    while (true) {
        SpinLock(&pmm.scrubLock);

        if (pmm.scrubStack != 0) {
            Frame_t frame = pmm.scrubStack->frame;
            size_t count = pmm.scrubStack->count;
            PopStack(&pmm.scrubStack);
            SpinUnlock(&pmm.scrubLock);

            PmmScrubBlock(frame, count);

            if (frame < 0x100) {
                SpinLock(&pmm.lowLock);
                PushStack(&pmm.lowStack, frame, count);
                SpinUnlock(&pmm.lowLock);
            } else {
                SpinLock(&pmm.normLock);
                PushStack(&pmm.normStack, frame, count);
                SpinUnlock(&pmm.normLock);
            }
        } else {
            SpinUnlock(&pmm.scrubLock);
            // -- TODO: nothing to do; wait for a message to do something else
        }
    }
}


//
// -- Complete the late initialization of the PMM
//    -------------------------------------------
void pmm_LateInit(void)
{
    KernelPrintf("Starting PMM Cleaner process\n");
    SchProcessCreate("PMM Cleaner", (Addr_t)PmmCleanProcess, GetAddressSpace(), PTY_LOW);
    KernelPrintf("PMM Late Initialization Complete!\n");
}


