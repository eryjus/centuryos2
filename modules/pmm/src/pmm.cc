/****************************************************************************************************************//**
*   @file               pmm.cc
*   @brief              Functions to handle the PMM management
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Feb-12
*   @since              v0.0.04
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*   This file contains the functions for the PMM to implement the Physical Memory Manager (PMM).  These functions
*   will be used in user space to allocate memory to the kernel, which which will then map the memory for the
*   individual requests.  That is an important distinction -- only the kernel should be interacting with the
*   PMM, no other processes should need access.  As a result, the PMM should be protected from interference.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Feb-12 | Initial |  v0.0.04 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "boot-interface.h"
#include "kernel-funcs.h"



/********************************************************************************************************************
* Some internal function prototypes
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Return_t PmmInitEarly(BootInterface_t *loaderInterface);
extern "C" Frame_t pmm_PmmAllocateAligned(bool low, int bitsAligned, size_t count);
extern "C" Return_t pmm_PmmReleaseFrame(Frame_t frame, size_t count);
extern "C" void pmm_LateInit(void);

#if DEBUG_ENABLED(pmm_PmmReleaseFrame) || DEBUG_ENABLED(pmm_PmmReleaseFrame) || IS_ENABLED(KERNEL_DEBUGGER)

#if IS_ENABLED(KERNEL_DEBUGGER)
    extern "C" void PmmDebugInit(void);
#else
#   define DbgOutput KernelPrintf
#endif

extern "C" void DumpPmm(void);

#endif


/****************************************************************************************************************//**
*   @typedef            PmmFrameInfo_t
*   @brief              Formalization of the PMM Frame Information Structure
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             PmmFrameInfo_t
*   @brief              This is the new PMM frame information structure -- contains info about this block of frames
*
*   This structure is overwritten on top of the start of the frame.  The frame is mapped to some PMM-specific pages
*   in order to maintain stacks.  To identify the frame number and it length (along with the previous and next
*   frames), this structure maps the elements.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct PmmFrameInfo_t {
    Frame_t frame;          //!< The frame number for the start of the block
    size_t count;           //!< The number of frames in this block
    Frame_t prev;           //!< The previous frame number on the stack \note 0 if top of the stack
    Frame_t next;           //!< The next frame number on the stack \note 0 if bottom of the stack
} PmmFrameInfo_t;



/****************************************************************************************************************//**
*   @typedef            Pmm_t
*   @brief              Formalization of the PMM Management Structure
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Pmm_t
*   @brief              This is the new PMM itself
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Pmm_t {
    AtomicInt_t framesAvail;        //!< this is the total number of frames available in the 3 stacks

    Spinlock_t lowLock;             //!< This lock protects lowStack
    PmmFrameInfo_t *lowStack;       //!< The stack of available frames in the low memory area (<1M)

    Spinlock_t normLock;            //!< This lock protects normStack
    PmmFrameInfo_t *normStack;      //!< The stack of available frames in the normal memory area (>=1M)

    Spinlock_t scrubLock;           //!< This lock protects scrubStack
    PmmFrameInfo_t *scrubStack;     //!< The stack of available frames that need to be sanitized

    Spinlock_t searchLock;          //!< This lock protects search
    PmmFrameInfo_t *search;         //!< used for searching the PMM for a frame

    // -- some additional locks for mapped address usage
    Spinlock_t insertLock;          //!< Lock that protects a temporary address for inserting a block
    Spinlock_t clearLock;           //!< Lock that protects the address used to clear a frame
} Pmm_t;



/****************************************************************************************************************//**
*   @var                Pmm_t
*   @brief              The actual PMM in memory
*///-----------------------------------------------------------------------------------------------------------------
Pmm_t pmm = { { 0 } };


#if DEBUG_ENABLED(pmm_PmmReleaseFrame) || DEBUG_ENABLED(pmm_PmmReleaseFrame) || IS_ENABLED(KERNEL_DEBUGGER)

/****************************************************************************************************************//**
*   @fn                 void DumpPmm(void)
*   @brief              Dump the contents of the PMM structures
*
*   Dump the PMM structures contents for debugging purposes.
*///-----------------------------------------------------------------------------------------------------------------
void DumpPmm(void)
{
    char buf[128];

    DbgOutput(ANSI_CLEAR ANSI_SET_CURSOR(0,0));
    DbgOutput(ANSI_ATTR_BOLD ANSI_FG_RED " Dumping PMM Structure:\n");
    DbgOutput("+----------------------------+--------------------------+\n");

    ksprintf(buf, "| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Number of frames available" ANSI_ATTR_NORMAL
            " | %-12ld             |\n", pmm.framesAvail);
    DbgOutput(buf);

    ksprintf(buf, "| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Low Lock State" ANSI_ATTR_NORMAL
            "             | %-8.8s                 |\n", pmm.lowLock.lock?"locked":"unlocked");
    DbgOutput(buf);

    ksprintf(buf, "| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Low Stack Address" ANSI_ATTR_NORMAL
            "          | %p         |\n", pmm.lowStack);
    DbgOutput(buf);

    if (pmm.lowStack && MmuIsMapped((Addr_t)pmm.lowStack)) {
        ksprintf(buf, "|   " ANSI_ATTR_BOLD ANSI_FG_BLUE "Low Stack TOS frame" ANSI_ATTR_NORMAL
                "      | %p         |\n", pmm.lowStack->frame);
        DbgOutput(buf);

        ksprintf(buf, "|   " ANSI_ATTR_BOLD ANSI_FG_BLUE "Low Stack TOS count" ANSI_ATTR_NORMAL
                "      | %-8d                 |\n", pmm.lowStack->count);
        DbgOutput(buf);
    }

    ksprintf(buf, "| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Normal Lock State" ANSI_ATTR_NORMAL
            "          | %-8.8s                 |\n", pmm.normLock.lock?"locked":"unlocked");
    DbgOutput(buf);

    ksprintf(buf, "| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Normal Stack Address" ANSI_ATTR_NORMAL
            "       | %p         |\n", pmm.normStack);
    DbgOutput(buf);

    if (pmm.normStack && MmuIsMapped((Addr_t)pmm.normStack)) {
        ksprintf(buf, "|   " ANSI_ATTR_BOLD ANSI_FG_BLUE "Normal Stack TOS frame" ANSI_ATTR_NORMAL
                "   | %p         |\n", pmm.normStack->frame);
        DbgOutput(buf);

        ksprintf(buf, "|   " ANSI_ATTR_BOLD ANSI_FG_BLUE "Normal Stack TOS count" ANSI_ATTR_NORMAL
                "   | %-8d                 |\n", pmm.normStack->count);
        DbgOutput(buf);
    }

    ksprintf(buf, "| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Scrub Lock State" ANSI_ATTR_NORMAL
            "           | %-8.8s                 |\n", pmm.scrubLock.lock?"locked":"unlocked");
    DbgOutput(buf);

    ksprintf(buf, "| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Scrub Stack Address" ANSI_ATTR_NORMAL
            "        | %p         |\n", pmm.scrubStack);
    DbgOutput(buf);

    if (pmm.scrubStack && MmuIsMapped((Addr_t)pmm.scrubStack)) {
        ksprintf(buf, "|   " ANSI_ATTR_BOLD ANSI_FG_BLUE "Scrub Stack TOS frame" ANSI_ATTR_NORMAL
                "    | %p         |\n", pmm.scrubStack->frame);
        DbgOutput(buf);

        ksprintf(buf, "|   " ANSI_ATTR_BOLD ANSI_FG_BLUE "Scrub Stack TOS count" ANSI_ATTR_NORMAL
                "    | %-8d                 |\n", pmm.scrubStack->count);
        DbgOutput(buf);
    }

    ksprintf(buf, "| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Search Lock State" ANSI_ATTR_NORMAL
            "          | %-8.8s                 |\n", pmm.searchLock.lock?"locked":"unlocked");
    DbgOutput(buf);

    ksprintf(buf, "| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Search Stack Address" ANSI_ATTR_NORMAL
            "       | %p         |\n", pmm.search);
    DbgOutput(buf);

    if (pmm.search && MmuIsMapped((Addr_t)pmm.search)) {
        ksprintf(buf, "|   " ANSI_ATTR_BOLD ANSI_FG_BLUE "Search Stack TOS frame" ANSI_ATTR_NORMAL
                "   | %p         |\n", pmm.search->frame);
        DbgOutput(buf);

        ksprintf(buf, "|   " ANSI_ATTR_BOLD ANSI_FG_BLUE "Search Stack TOS count" ANSI_ATTR_NORMAL
                "   | %-8d                 |\n", pmm.search->count);
        DbgOutput(buf);
    }

    DbgOutput("+----------------------------+--------------------------+\n");

}

#endif



/****************************************************************************************************************//**
*   @fn                 static void PushStack(PmmFrameInfo_t **stack, Frame_t frame, size_t count)
*   @brief              Push frames on a stack (must hold the stack lock to call)
*
*   Push a block of frames into the stack provided.
*
*   @param              stack           Pointer to the pointer to the top of the stack (which will be changed)
*   @param              frame           The frame that starts the block to push
*   @param              count           The number of frames in this block
*///-----------------------------------------------------------------------------------------------------------------
static void PushStack(PmmFrameInfo_t **stack, Frame_t frame, size_t count)
{
    SpinLock(&pmm.insertLock);
    MmuMapPage(TEMP_INSERT, frame, PG_WRT);
    volatile PmmFrameInfo_t *wrk = (PmmFrameInfo_t *)TEMP_INSERT;

    wrk->frame = frame;
    wrk->count = count;
    wrk->prev = 0;

    if (*stack) {
        wrk->next = (*stack)->frame;
        (*stack)->prev = frame;

        MmuUnmapPage((Addr_t)(*stack));
    } else {
        wrk->next = 0;

        if (stack == &pmm.lowStack) *stack = (PmmFrameInfo_t *)LOW_STACK;
        else if (stack == &pmm.normStack) *stack = (PmmFrameInfo_t *)NORM_STACK;
        else if (stack == &pmm.scrubStack) *stack = (PmmFrameInfo_t *)SCRUB_STACK;
        else KernelPrintf("PushStack(): Unable to determine on which stack to push\n");
    }

    MmuUnmapPage(TEMP_INSERT);
    SpinUnlock(&pmm.insertLock);

    // -- finally, push the new node
    MmuMapPage((Addr_t)(*stack), frame, PG_WRT);
}



/****************************************************************************************************************//**
*   @fn                 static void PopStack(PmmFrameInfo_t **stack)
*   @brief              Pop the last frame off the stack (must hold the stack lock to call)
*
*   Pop a block of frames from the stack provided.
*
*   @param              stack           Pointer to the pointer to the top of the stack (which will be changed)
*///-----------------------------------------------------------------------------------------------------------------
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



/****************************************************************************************************************//**
*   @fn                 static void PmmScrubFrame(Frame_t frame)
*   @brief              Scrub a frame, clearing its contents
*
*   Sanitize a frame by writing zeros to the entire frame
*
*   @param              frame           The frame to clear
*///-----------------------------------------------------------------------------------------------------------------
static void PmmScrubFrame(Frame_t frame)
{
    SpinLock(&pmm.clearLock);

#if DEBUG_ENABLED(PmmScrubFrame)

    KernelPrintf("!!!!! PmmScrubFrame() preparing to map a page\n");

#endif

    MmuMapPage(CLEAR_ADDR, frame, PG_WRT);

#if DEBUG_ENABLED(PmmScrubFrame)

    KernelPrintf("!!!!! PmmScrubFrame() page mapped\n");
    MmuDump(CLEAR_ADDR);

#endif

    uint64_t *wrk = (uint64_t *)CLEAR_ADDR;

    for (int i = 0; i < PAGE_SIZE / sizeof(uint64_t); i ++) wrk[i] = 0;

    MmuUnmapPage(CLEAR_ADDR);

    SpinUnlock(&pmm.clearLock);
}



/****************************************************************************************************************//**
*   @fn                 static void PmmScrubBlock(Frame_t start, size_t count)
*   @brief              Scrub a block of frames, clearing its contents
*
*   Sanitize a block of frame by writing zeros to the entire block
*
*   @param              start           The first frame to clear
*   @param              count           The number of frames to clear
*///-----------------------------------------------------------------------------------------------------------------
static void PmmScrubBlock(Frame_t start, size_t count)
{
    for (size_t i = start; i < start + count; i ++) PmmScrubFrame(i);
}



/****************************************************************************************************************//**
*   @fn                 static Frame_t PmmDoRemoveFrame(PmmFrameInfo_t **stack, bool scrub)
*   @brief              Remove a Frame from a block (the stack lock must be held to call)
*
*   Remove a single block from the top of the stack.  If the block has no frames left, pop that block from the
*   stack.
*
*   @param              start           The first frame to clear
*   @param              count           The number of frames to clear
*///-----------------------------------------------------------------------------------------------------------------
static Frame_t PmmDoRemoveFrame(PmmFrameInfo_t **stack, bool scrub)
{
    Frame_t rv = 0;         // assume we will not find anything

#if DEBUG_ENABLED(PmmDoRemoveFrame)

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

#if DEBUG_ENABLED(PmmDoRemoveFrame)

    KernelPrintf(".. Found the frame to use: %p\n", rv);

#endif


    // -- scrub the frame if requested
    if (scrub) PmmScrubFrame(rv);

#if DEBUG_ENABLED(PmmDoRemoveFrame)

    KernelPrintf(".. all good!  Returning frame %p\n", rv);

#endif

    return rv;
}



/****************************************************************************************************************//**
*   @fn                 Return_t PmmInitEarly(BootInterface_t *loaderInterface)
*   @brief              Complete the initialization required for the PMM
*
*   Perform the initialization required to put the PMM in service.  Blocks will be inserted from the MultiBoot
*   memory map into the scrub stack, with a reasonable section set aside for low memory and the early PMM.  The
*   Butler will then add additional memory blocks to the PMM once it can safely identify what is still available.
*   Note that this function also resets the PMM Allocator at the end of this function, putting itself in charge
*   of managing the PMM.  With all the available frames on the Scrub stack, the PMM will scrub any frame before
*   allowing it to be allocated and returned to the caller.
*
*   @param              loaderInterface     The available hardware interface from the loader
*
*   @returns            Whether the module should be loaded or not
*
*   @retval             0               The PMM is always successfully loaded
*///-----------------------------------------------------------------------------------------------------------------
Return_t PmmInitEarly(BootInterface_t *loaderInterface)
{
    ProcessInitTable();

#if DEBUG_ENABLED(PmmInitEarly)

    KernelPrintf("PmmInitEarly(): Starting initialization\n");

#endif

    for (int i = 0; i < MAX_MEM; i ++) {
        Frame_t start = loaderInterface->memBlocks[i].start >> 12;
        Frame_t end = loaderInterface->memBlocks[i].end >> 12;

        if (start == 0 && end == 0) continue;
        if (start < loaderInterface->nextEarlyFrame && end <= loaderInterface->nextEarlyFrame) continue;
        if (start < loaderInterface->nextEarlyFrame) start = loaderInterface->nextEarlyFrame + 0x100;

        Addr_t size = end - start;
        MmuMapPage(TEMP_MAP, start, PG_WRT);

        PmmFrameInfo_t *info = (PmmFrameInfo_t *)TEMP_MAP;
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
            MmuMapPage((Addr_t)SCRUB_STACK, info->frame, PG_WRT);
            pmm.scrubStack = (PmmFrameInfo_t *)SCRUB_STACK;
        }

        MmuUnmapPage(TEMP_MAP);
    }

    SetInternalHandler(INT_PMM_ALLOC, (Addr_t)pmm_PmmAllocateAligned, GetAddressSpace(), 0);

#if DEBUG_ENABLED(PmmInitEarly)

    KernelPrintf("PmmInitEarly(): Initialization complete\n");
    DumpPmm();

#endif

    return 0;   // will be loaded
}



/****************************************************************************************************************//**
*   @fn                 static Frame_t PmmAllocate(bool low)
*   @brief              Allocate a single frame (normal or low)
*
*   Allocate a frame from one of:
*   * The normal memory stack (when low == false)
*   * The scrub stack (when low == false && no normal memory available)
*   * The low memory stack (when low == true || no other memory available)
*
*   @param              low             Whether the frame should be from low memory or not
*
*   @returns            The frame allocated
*
*   @retval             -ENOMEM         When there is no physical memory left
*   @retval             frame           The frame allocated
*///-----------------------------------------------------------------------------------------------------------------
static Frame_t PmmAllocate(bool low)
{
    Frame_t rv = 0;         // assume we will not find anything


    //
    // -- check the normal stack for a frame to allocate
    //    ----------------------------------------------
    if (!low) {
#if DEBUG_ENABLED(PmmAllocate)

        KernelPrintf("Allocating a single frame from the normal stack\n");

#endif

        SpinLock(&pmm.normLock);
        rv = PmmDoRemoveFrame(&pmm.normStack, false);
        SpinUnlock(&pmm.normLock);

#if DEBUG_ENABLED(PmmAllocate)

        KernelPrintf("Frame Allocated: %p\n", rv);

#endif
    }

    if (rv != 0) return rv;


    //
    // -- check the scrub queue for a frame to allocate
    //    ---------------------------------------------
    if (!low) {

#if DEBUG_ENABLED(PmmAllocate)

        KernelPrintf("Allocating a single frame from the scrub stack\n");

#endif

        SpinLock(&pmm.scrubLock);
        rv = PmmDoRemoveFrame(&pmm.scrubStack, true);       // -- scrub the frame when it is removed
        SpinUnlock(&pmm.scrubLock);

#if DEBUG_ENABLED(PmmAllocate)

        KernelPrintf("Frame Allocated: %p\n", rv);

#endif
    }
    if (rv != 0) return rv;


    //
    // -- check the low stack for a frame to allocate
    //    -------------------------------------------

#if DEBUG_ENABLED(PmmAllocate)

    KernelPrintf("Allocating a single frame from the low stack\n");

#endif

    SpinLock(&pmm.lowLock);
    rv = PmmDoRemoveFrame(&pmm.lowStack, false);
    SpinUnlock(&pmm.lowLock);

#if DEBUG_ENABLED(PmmAllocate)

    KernelPrintf("Frame Allocated: %p\n", rv);

#endif

    if (rv != 0) return rv;

    return -ENOMEM;
}



/****************************************************************************************************************//**
*   @fn                 static Frame_t PmmSplitBlock(PmmFrameInfo_t **stack, Frame_t frame, size_t blockSize, \
*                       Frame_t atFrame, size_t count)static Frame_t PmmAllocate(bool low)
*   @brief              Split the block as needed to pull out the proper alignment and size of frames
*
*   Allocate a frame from one of:
*   * The normal memory stack (when low == false)
*   * The scrub stack (when low == false && no normal memory available)
*   * The low memory stack (when low == true || no other memory available)
*
*   @param              stack           Pointer to a pointer to the stack from which the block will be pulled
*   @param              frame           The frame number that starts the block required to be split
*   @param              blockSize       The number of frames in the block required to be split
*   @param              atFrame         The frame number at which the block will be split
*   @param              count           The number of frames required in the resulting allocation
*
*   @returns            The frame starting the allocation (atFrame)
*///-----------------------------------------------------------------------------------------------------------------
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



/****************************************************************************************************************//**
*   @fn                 Frame_t PmmDoAllocAlignedFrames(PmmFrameInfo_t **pStack, const size_t count, \
*                       const size_t bitAlignment)
*   @brief              The working to find a frame that is properly aligned and allocate multiple contiguous frames
*
*   Searches the stack specified for a block of memory large enough to hold the block required and at the
*   specified alignment.
*
*   @param              pStack          Pointer to a pointer to the stack to search
*   @param              count           The number of frames required in the resulting allocation
*   @param              bitAlignment    The number of bits on which the allocation needs to be aligned
*
*   @returns            The frame starting the allocation, -ENOMEM if no unable to allocate the frame
*
*   @retval             -ENOMEM         When there is no physical memory left
*   @retval             frame           The frame allocated
*///-----------------------------------------------------------------------------------------------------------------
Frame_t PmmDoAllocAlignedFrames(PmmFrameInfo_t **pStack, const size_t count, const size_t bitAlignment)
{
#if DEBUG_ENABLED(PmmDoAllocAlignedFrames)

    KernelPrintf("Allocating aligned frame(s)\n");

#endif

    //
    // -- start by determining the bits we cannot have enabled when we evaluate a frame
    //    -----------------------------------------------------------------------------
    Frame_t frameBits = ~(((Frame_t)-1) << (bitAlignment<12?0:bitAlignment-12));
    Frame_t rv = -ENOMEM;

    if (!MmuIsMapped((Addr_t)*pStack)) return -ENOMEM;


    // -- Interrupts are already disabled before we get here
#if DEBUG_ENABLED(PmmDoAllocAlignedFrames)

    KernelPrintf(".. Attempting the search lock\n");

#endif

    SpinLock(&pmm.searchLock); {
#if DEBUG_ENABLED(PmmDoAllocAlignedFrames)

        KernelPrintf(".. Lock obtained\n");

        KernelPrintf("Some interesting facts before MmuMapPage:\n");
        KernelPrintf(".. pmm.search is %p\n", pmm.search);
        KernelPrintf(".. pStack is %p (%s)\n", pStack, MmuIsMapped((Addr_t)pStack)?"mapped":"not mapped");
        KernelPrintf(".. (*pStack) is %p\n", *pStack);
        KernelPrintf(".. (*pStack)->frame is %p\n", pStack?(*pStack)->frame:0);

#endif

        MmuMapPage((Addr_t)pmm.search, (*pStack)->frame, PG_WRT);

#if DEBUG_ENABLED(PmmDoAllocAlignedFrames)

        KernelPrintf(".. Page mapped\n");

#endif

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

#if DEBUG_ENABLED(PmmDoAllocAlignedFrames)

    KernelPrintf("Aligned PMM Allocation is finally returning frame %x\n", rv);

#endif

    return rv;
}



/****************************************************************************************************************//**
*   @fn                 Frame_t pmm_PmmAllocateAligned(bool low, int bitsAligned, size_t count)
*   @brief              Allocate aligned frame(s)
*
*   The syscall target to allocate frames.  All allocations are at least 1 frame and aligned to 12 bits.
*   If those conditions are met (1 frame aligned at 12 bits), then the trivial \ref PmmAllocate functino
*   is used.
*
*   @param              low             Whether to allocate low memory
*   @param              bitsAligned     The alignment of the allocation
*   @param              count           The number of frames to allocate in a block
*
*   @returns            The frame starting the allocation, -ENOMEM if no unable to allocate the frame
*
*   @retval             -ENOMEM         When there is no physical memory left
*   @retval             frame           The frame allocated
*///-----------------------------------------------------------------------------------------------------------------
Frame_t pmm_PmmAllocateAligned(bool low, int bitsAligned, size_t count)
{
#if DEBUG_ENABLED(pmm_PmmAllocateAligned)

    KernelPrintf("Internal Function to allocate aligned frames (pmm_PmmAllocateAligned)\n");
    KernelPrintf(".. low = %s; alignment bits = %d; count = %d\n", low?"true":"false", bitsAligned, count);

#endif

    if (bitsAligned < 12) bitsAligned = 12;
    if (count == 1 && bitsAligned == 12) {
        Frame_t rv = PmmAllocate(low);

#if DEBUG_ENABLED(pmm_PmmAllocateAligned)

        KernelPrintf(".. Returning back to the user frame %p\n", rv);

#endif

        return rv;
    }

#if DEBUG_ENABLED(pmm_PmmAllocateAligned)

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



/****************************************************************************************************************//**
*   @fn                 Return_t pmm_PmmReleaseFrame(Frame_t frame, size_t count)
*   @brief              Release a frame
*
*   Quickly release a frame or block of frames by pushing the frames onto the scrub stack and exiting.
*
*   @param              frame           The start of the block of frames to release
*   @param              count           The number of frames to release in a block
*
*   @returns            0
*///-----------------------------------------------------------------------------------------------------------------
Return_t pmm_PmmReleaseFrame(Frame_t frame, size_t count)
{
    SpinLock(&pmm.scrubLock);
    PushStack(&pmm.scrubStack, frame, count);
    AtomicAdd(&pmm.framesAvail, count);
    SpinUnlock(&pmm.scrubLock);

#if DEBUG_ENABLED(pmm_PmmReleaseFrame)

    DumpPmm();

#endif

//    MessageQueueSend(butlerMsgq, BUTLER_CLEAN_PMM, 0, 0);
    return 0;
}



/****************************************************************************************************************//**
*   @fn                 void PmmCleanProcess(void)
*   @brief              Clean the blocks on the scrub stack
*
*   This process runs to clean blocks of frames from the scrub stack and insert them onto the proper low or normal
*   stack.
*///-----------------------------------------------------------------------------------------------------------------
void PmmCleanProcess(void)
{
#if DEBUG_ENABLED(PmmCleanProcess)

    KernelPrintf("Starting the PMM Cleaner\n");

#endif

    while (true) {
        SpinLock(&pmm.scrubLock);

#if DEBUG_ENABLED(PmmCleanProcess)

        KernelPrintf("Checking for a mapped scrub Stack\n");
        MmuDump((Addr_t)pmm.scrubStack);

#endif

        if (MmuIsMapped((Addr_t)pmm.scrubStack) == true) {
#if DEBUG_ENABLED(PmmCleanProcess)

            KernelPrintf(".. found a block to clean\n");

#endif

            Frame_t frame = pmm.scrubStack->frame;
            size_t count = pmm.scrubStack->count;
            PopStack(&pmm.scrubStack);
            SpinUnlock(&pmm.scrubLock);

#if DEBUG_ENABLED(PmmCleanProcess)

            KernelPrintf(".. scrubbing the block: %p for %d blocks\n", frame, count);

#endif

            PmmScrubBlock(frame, count);

            if (frame < 0x100) {
#if DEBUG_ENABLED(PmmCleanProcess)

                KernelPrintf(".. inserting into the low Stack\n");

#endif

                SpinLock(&pmm.lowLock);
                PushStack(&pmm.lowStack, frame, count);
                SpinUnlock(&pmm.lowLock);

#if DEBUG_ENABLED(PmmCleanProcess)

                KernelPrintf(".... Done!\n");

#endif
            } else {
#if DEBUG_ENABLED(PmmCleanProcess)

                KernelPrintf(".. inserting into the normal stack\n");

#endif

                SpinLock(&pmm.normLock);
                PushStack(&pmm.normStack, frame, count);
                SpinUnlock(&pmm.normLock);

#if DEBUG_ENABLED(PmmCleanProcess)

                KernelPrintf(".... Done!\n");

#endif
            }
        } else {
#if DEBUG_ENABLED(PmmCleanProcess)

            KernelPrintf(".. nothing left to do\n");

#endif

            SpinUnlock(&pmm.scrubLock);
            SchProcessBlock(PRC_MSGW);
        }
    }
}



/****************************************************************************************************************//**
*   @fn                 void pmm_LateInit(void)
*   @brief              Complete the late initialization of the PMM
*
*   Complete the late initialization by starting a new process to clean the scrub stack.
*///-----------------------------------------------------------------------------------------------------------------
void pmm_LateInit(void)
{
#if DEBUG_ENABLED(pmm_LateInit)

    KernelPrintf("Starting PMM Cleaner process\n");

#endif

    SchProcessCreate("PMM Cleaner", (Addr_t)PmmCleanProcess, GetAddressSpace(), PTY_LOW);

#if DEBUG_ENABLED(pmm_LateInit)

    KernelPrintf("PMM Late Initialization Complete!\n");
#endif

#if IS_ENABLED(KERNEL_DEBUGGER)
    PmmDebugInit();
#endif
}



#if IS_ENABLED(KERNEL_DEBUGGER)
#include "debugger.h"
#include "stacks.h"


//
// -- here is the debugger menu & function ecosystem
//    ----------------------------------------------
DbgState_t pmmStates[] = {
    {   // -- state 0
        .name = "pmm",
        .transitionFrom = 0,
        .transitionTo = 1,
    },
    {   // -- state 1 (status)
        .name = "status",
        .function = (Addr_t)DumpPmm,
    },
};


DbgTransition_t pmmTrans[] = {
    {   // -- transition 0
        .command = "status",
        .alias = "s",
        .nextState = 1,
    },
    {   // -- transition 1
        .command = "exit",
        .alias = "x",
        .nextState = -1,
    },
};


DbgModule_t pmmModule = {
    .name = "pmm",
    .addrSpace = GetAddressSpace(),
    .stack = 0,     // -- needs to be handled during late init
    .stateCnt = sizeof(pmmStates) / sizeof (DbgState_t),
    .transitionCnt = sizeof(pmmTrans) / sizeof (DbgTransition_t),
    .list = {&pmmModule.list, &pmmModule.list},
    .lock = {0},
    // -- it does not matter what we put for .states and .transitions; will be replaced in debugger
};



/****************************************************************************************************************//**
*   @fn                 void PmmDebugInit(void)
*   @brief              Initialize the debugger module structure
*
*   Initialize the debugger module for the PMM
*///-----------------------------------------------------------------------------------------------------------------
void PmmDebugInit(void)
{
    extern Addr_t __stackSize;

    pmmModule.stack = StackFind();
    for (Addr_t s = pmmModule.stack; s < pmmModule.stack + __stackSize; s += PAGE_SIZE) {
        MmuMapPage(s, PmmAlloc(), PG_WRT);
    }
    pmmModule.stack += __stackSize;

    DbgRegister(&pmmModule, pmmStates, pmmTrans);
}



#endif


