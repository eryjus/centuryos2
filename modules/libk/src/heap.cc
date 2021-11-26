//===================================================================================================================
//
//  heap.cc -- Kernel Heap implementation
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


#include "types.h"
#include "cpu.h"
#include "kernel-funcs.h"
#include "heap.h"



//
// -- Get the arch-specific components
//    --------------------------------
#if __has_include("arch/heap-arch.h")
#include "arch/heap-arch.h"
#endif


//
// -- forward declare the OrderedList structure
//    -----------------------------------------
struct OrderedList_t;


//
// -- This is the heap block header, used to manage a block of memory in the heap
//    ---------------------------------------------------------------------------
typedef struct KHeapHeader_t {
    union {
        MagicHole_t mhStruct;                   // magic number and hold flag structure
        Addr_t magicHole;                       // this is the aggregate of the bit fields
    } _magicUnion;

    struct OrderedList_t *entry;                // pointer to the OrderedList entry if hole; NULL if allocated
    size_t size;                                // this size includes the size of the header and footer
} __attribute__((packed)) KHeapHeader_t;


//
// -- This is the beap block footer, used in conjunction with the heap header to makage the heap memory
//    -------------------------------------------------------------------------------------------------
typedef struct KHeapFooter_t {
    union {
        MagicHole_t mhStruct;                   // magic number and hold flag structure
        Addr_t magicHole;                       // this is the aggregate of the bit fields
    } _magicUnion;

    KHeapHeader_t *hdr;                        // pointer back to the header
} __attribute__((packed)) KHeapFooter_t;


//
// -- This is a compare function prototype declaration for ordering blocks
//    --------------------------------------------------------------------
typedef int (*cmpFunc)(KHeapHeader_t *, KHeapHeader_t *);


//
// -- The heap is implemented as an ordered list for a bet-fit implementation
//    -----------------------------------------------------------------------
typedef struct OrderedList_t {
    KHeapHeader_t *block;                      // pointer to the block of heap memory
    size_t size;                               // the size of the memory pointed to
    struct OrderedList_t *prev;                // pointer to the previous entry
    struct OrderedList_t *next;                // pointer to the next entry
} OrderedList_t;


//
// -- This is the heap control structure, maintianing the heap integrity
//    ------------------------------------------------------------------
typedef struct KHeap_t {
    bool initialized;                           // has the heap been initialized?
    OrderedList_t *heapMemory;                  // the start of all heap memory lists < 512 bytes
    OrderedList_t *heap512;                     // the start of heap memory >= 512 bytes
    OrderedList_t *heap1K;                      // the start of heap memory >= 1K bytes
    OrderedList_t *heap4K;                      // the start of heap memory >= 4K bytes
    OrderedList_t *heap16K;                     // the start of heap memory >= 16K bytes
    Byte_t *strAddr;                            // the start address of the heap
    Byte_t *endAddr;                            // the ending address of the heap
    Byte_t *maxAddr;                            // the max address to which the heap can grow
} KHeap_t;



//
// -- This is how much of the heap we will allocate at compile time.  This will really be frames that will be moved
//    during initialization.  This is 64K.
//    -------------------------------------------------------------------------------------------------------------
#define INITIAL_HEAP        (4096*16)


//
// -- some local and global variables
//    -------------------------------
extern Addr_t __heapStart, __heapEnd;

Spinlock_t heapLock = {0};
const Addr_t heapStart = __heapStart;              // this is the start in virtual address space
const Addr_t heapEnd = __heapEnd;

OrderedList_t fixedList[ORDERED_LIST_STATIC];
bool fixedListUsed = 0;

static KHeap_t _heap = {
    .initialized = false,
    .heapMemory = NULL,
    .heap512 = NULL,
    .heap1K = NULL,
    .heap4K = NULL,
    .heap16K = NULL,
    .strAddr = (Byte_t *)(heapStart),
    .endAddr = (Byte_t *)(heapStart + INITIAL_HEAP),
    .maxAddr = (Byte_t *)(heapEnd),
};

KHeap_t *kHeap = &_heap;


//
// -- Panic the kernel as the result of a heap error
//    ----------------------------------------------
static void HeapError(const char *from, const char *desc)
{
    DisableInt();
    KernelPrintf("Heap Error!!! %s - %s\n", from, desc);
    while (true) {
        __asm volatile ("hlt");
    }
}


//
// -- Validate a heap header block to ensure it has not been overrun
//    --------------------------------------------------------------
static void HeapValidateHdr(KHeapHeader_t *hdr, const char *from)
{
    KHeapFooter_t *ftr;

    if (!hdr) {
        HeapError(from, "Unable to validate NULL header");
    }

    ftr = (KHeapFooter_t *)((char *)hdr + hdr->size - sizeof(KHeapFooter_t));

    if ((hdr->_magicUnion.magicHole & HEAP_CHECK_MASK) != HEAP_MAGIC) {
        HeapError(from, "Invalid Heap Header Magic Number");
    }

    if ((ftr->_magicUnion.magicHole & HEAP_CHECK_MASK) != HEAP_MAGIC) {
        HeapError(from, "Invalid Heap Footer Magic Number");
    }

    if (hdr->_magicUnion.magicHole != ftr->_magicUnion.magicHole) {
        HeapError(from, "Header/Footer Magic Number/Hole mismatch");
    }

    if (hdr->_magicUnion.mhStruct.isHole == 1 && hdr->entry == 0) {
        HeapError(from, "Heap hole has no ordered list entry");
    }

    if (hdr->_magicUnion.mhStruct.isHole == 0 && hdr->entry != 0) {
        HeapError(from, "Heap allocated block has an ordered list entry");
    }

    if (hdr->entry && hdr->entry->block != hdr) {
        HeapError(from, "Entry does not point to this header");
    }

    if (hdr->entry && hdr->entry->size != hdr->size) {
        HeapError(from, "Header/Entry size mismatch");
    }
}


//
// -- Check the heap structure
//    ------------------------
static void HeapValidatePtr(const char *from)
{
    if (!kHeap->heapMemory) {
        HeapError(from, "Start of heapMemory is empty");
    }

    if (!kHeap->heapMemory) return;

    HeapValidateHdr(kHeap->heapMemory->block, from);
}


//
// -- Add an ordered list entry to the heap structures
//    ------------------------------------------------
static void HeapAddToList(OrderedList_t *entry)
{
    OrderedList_t *wrk, *sav = 0;
    size_t size;

    if (!assert(entry != NULL)) HeapError("NULL entry in HeapAddToList()", "");
    HeapValidateHdr(entry->block, "HeapAddToList()");
    // cannot validate heap ptrs as may be empty

    size = entry->size;

    // assume that we are starting at the beginning
    wrk = kHeap->heapMemory;

    if (wrk) {
        if (size >= 512 && kHeap->heap512) wrk = kHeap->heap512;
        if (size >= 1024 && kHeap->heap1K) wrk = kHeap->heap1K;
        if (size >= 4096 && kHeap->heap4K) wrk = kHeap->heap4K;
        if (size >= 16384 && kHeap->heap16K) wrk = kHeap->heap16K;
    } else {
        // special case, nothing in the Ordered List; make it right and leave
        kHeap->heapMemory = entry;
        entry->next = entry->prev = 0;

        if (size >= 512) kHeap->heap512 = entry;
        if (size >= 1024) kHeap->heap1K = entry;
        if (size >= 4096) kHeap->heap4K = entry;
        if (size >= 16384) kHeap->heap16K = entry;

        goto out;
    }

    // in theory, wrk is now optimized for a faster search for the right size
    while (wrk) {    // while we have something to work with...
        if (wrk->size < size) {
            sav = wrk;
            wrk = wrk->next;
            continue;
        }

        // at this point, we need to insert before wrk
        entry->next = wrk;
        entry->prev = wrk->prev;
        if (entry->next) entry->next->prev = entry;
        if (entry->prev) entry->prev->next = entry;

        break;
    }

    // check if we need to add to the end -- special case
    if (!wrk) {
        sav->next = entry;
        entry->prev = sav;
        entry->next = 0;
    }

    // entry inserted; now fix-up the optimized pointers; start with NULLs
    if (!kHeap->heap512 && size >= 512) kHeap->heap512 = entry;
    if (!kHeap->heap1K && size >= 1024) kHeap->heap1K = entry;
    if (!kHeap->heap4K && size >= 4096) kHeap->heap4K = entry;
    if (!kHeap->heap16K && size >= 16384) kHeap->heap16K = entry;

    // fixup the pointer for >= 512 bytes
    if (kHeap->heap512) {
        if (kHeap->heap512->prev && kHeap->heap512->prev->size >= 512) {
            kHeap->heap512 = kHeap->heap512->prev;
        }
    }

    // fixup the pointer for >= 1024 bytes
    if (kHeap->heap1K) {
        if (kHeap->heap1K->prev && kHeap->heap1K->prev->size >= 1024) {
            kHeap->heap1K = kHeap->heap1K->prev;
        }
    }

    // fixup the pointer for >= 4096 bytes
    if (kHeap->heap4K) {
        if (kHeap->heap4K->prev && kHeap->heap4K->prev->size >= 4096) {
            kHeap->heap4K = kHeap->heap4K->prev;
        }
    }

    // fixup the pointer for >= 16384 bytes
    if (kHeap->heap16K) {
        if (kHeap->heap16K->prev && kHeap->heap16K->prev->size >= 16384) {
            kHeap->heap16K = kHeap->heap16K->prev;
        }
    }

out:
    HeapValidatePtr("HeapAddToList()");
    HeapValidateHdr(entry->block, "HeapAddToList() at exit");
}



//
// -- Execute some sanity checks on the overall heap structures
//    ---------------------------------------------------------
static void HeapCheckHealth(void)
{
    KHeapHeader_t *block;
    KHeapFooter_t *ftr;

    int numBlocks = 0;
    int numAlloc = 0;
    int numFree = 0;
    int numCorrupt = 0;
    int ttlAlloc = 0;
    int ttlFree = 0;
    int largeSize = 0;

    block = (KHeapHeader_t *)kHeap->strAddr;

    // guaranteed to be at least 1 block
    do {
        ftr = (KHeapFooter_t *)((char*)block + block->size - sizeof(KHeapFooter_t));

        // count the number of blocks regardless of status
        numBlocks ++;

        // now determine if block is corrupt
        if ((block->_magicUnion.magicHole & HEAP_CHECK_MASK) != HEAP_MAGIC ||
                (ftr->_magicUnion.magicHole & HEAP_CHECK_MASK) != HEAP_MAGIC) {
            numCorrupt ++;
        } else if (block->_magicUnion.magicHole != ftr->_magicUnion.magicHole) {
            numCorrupt ++;
        } else if (ftr->hdr != block) {
            numCorrupt ++;
        // now check for free
        } else if (block->_magicUnion.mhStruct.isHole == 1) {
            if (block->entry != 0) {
                numFree ++;
                ttlFree += block->size;

                if (block->size > largeSize) {
                    largeSize = block->size;
                }
            } else {
                numCorrupt ++;
            }
        // now check for alloc
        } else if (block->_magicUnion.mhStruct.isHole == 0) {
            if (block->entry == 0) {
                numAlloc ++;
                ttlAlloc += block->size;
            } else {
                numCorrupt ++;
            }
        }

        block = (KHeapHeader_t *)((char *)block + block->size);
    } while ((Byte_t *)block < kHeap->endAddr);

    if (!numCorrupt) return;
    else while (1);
}


//
// -- Calculate the adjustment needed to align to a page
//    --------------------------------------------------
static size_t HeapCalcPageAdjustment(OrderedList_t *entry)
{
    Addr_t wrkPtr;

    assert(entry != NULL);

    wrkPtr = (Addr_t)entry->block + sizeof(KHeapHeader_t);

    // if not a page aligned block, align it
    if (wrkPtr & 0x00000fff) {
        wrkPtr = (wrkPtr & 0xfffff000) + 0x1000; //! next page
    }

    return wrkPtr - sizeof(KHeapHeader_t);
}


//
// -- Find the best fit hole in the list of holes
//    -------------------------------------------
static OrderedList_t *HeapFindHole(size_t adjustedSize, bool align)
{
    OrderedList_t *wrk = NULL;
    size_t wrkSize;

    // First determine the right starting point for searching.
    if (adjustedSize < 512) wrk = kHeap->heapMemory;
    if (adjustedSize >= 512 && adjustedSize < 1024) wrk = kHeap->heap512;
    if (adjustedSize >= 1024 && adjustedSize < 4096) wrk = kHeap->heap1K;
    if (adjustedSize >= 4096 && adjustedSize < 16384) wrk = kHeap->heap4K;
    if (adjustedSize >= 16384) wrk = kHeap->heap16K;

    // in theory, wrk is now optimized for a faster search for the right size
    while (wrk) {    // while we have something to work with...
        if (wrk->size < adjustedSize) {
            wrk = wrk->next;
            continue;
        }

        // first entry of sufficient size and we are not aligning; use it
        if (wrk->size >= adjustedSize && !align) return wrk;

        // at this point, guaranteed to be looking for an aligned block
        // find the real block location; now, calculate the new block size
        wrkSize = wrk->size - (HeapCalcPageAdjustment(wrk) - (Addr_t)wrk->block);

        // check if we have overrun the block
        if (wrkSize <= 0) {
            wrk = wrk->next;
            continue;
        }

        // wrkSize now has the available memory for the block after adjusting
        // for page alignment; remember we pulled the size of the header out
        // check for a fit
        if (wrkSize >= adjustedSize - sizeof(KHeapHeader_t)) return wrk;

        // not big enough yet, move on
        wrk = wrk->next;
    }

    // no memory to allocate
    return 0;
}


//
// -- Expand the heap size (we have the heap lock)
//    --------------------------------------------
static size_t HeapExpand(void)
{
    // TODO: remove the following line
    return 0;


    if (!assert_msg(kHeap->endAddr < kHeap->maxAddr, "All Heap memory allocated; unable to create more")) {
        return 0;
    }

#if DEBUG_ENABLED(HeapExpand)

    KernelPrintf("Expanding heap...\n");

#endif

    size_t rv = 0;
    Byte_t *newEnd = kHeap->endAddr + HEAP_SIZE_INCR;

    if (newEnd > kHeap->maxAddr) newEnd = kHeap->maxAddr;

#if DEBUG_ENABLED(HeapExpand)

    KernelPrintf(".. new end will be %p (%d additional pages)\n", newEnd, (newEnd - kHeap->endAddr) >> 12);

#endif

    while (kHeap->endAddr < newEnd) {
#if DEBUG_ENABLED(HeapExpand)

        KernelPrintf(".. getting a frame...\n");

#endif

        Frame_t frame = PmmAlloc();

#if DEBUG_ENABLED(HeapExpand)

        KernelPrintf(".. mapping\n");

#endif

        MmuMapPage((Addr_t)kHeap->endAddr, frame, PG_WRT);

#if DEBUG_ENABLED(HeapExpand)

        KernelPrintf(".. done\n");

#endif

        kHeap->endAddr += PAGE_SIZE;
        rv += PAGE_SIZE;
    }

#if DEBUG_ENABLED(HeapExpand)

    KernelPrintf("Heap expanded by %d bytes\n", rv);

#endif

    return rv;
}


//
// -- Remove an entry from the Ordered List
//    -------------------------------------
static void HeapRemoveFromList(OrderedList_t *entry)
{
    if (!assert(entry != NULL)) HeapError("NULL entry in HeapRemoveFromList()", "");
    HeapValidateHdr(entry->block, "HeapRemoveFromList()");

    if (kHeap->heapMemory == entry) {
        kHeap->heapMemory = kHeap->heapMemory->next;
    }

    if (kHeap->heap512 == entry) {
        kHeap->heap512 = kHeap->heap512->next;
    }

    if (kHeap->heap1K == entry) {
        kHeap->heap1K = kHeap->heap1K->next;
    }

    if (kHeap->heap4K == entry) {
        kHeap->heap4K = kHeap->heap4K->next;
    }

    if (kHeap->heap16K == entry) {
        kHeap->heap16K = kHeap->heap16K->next;
    }

    if (entry->next) entry->next->prev = entry->prev;
    if (entry->prev) entry->prev->next = entry->next;

    entry->next = entry->prev = 0;
}


//
// -- Release an entry from the ordered list
//    --------------------------------------
static void HeapReleaseEntry(OrderedList_t *entry)
{
    if (!assert(entry != NULL)) HeapError("NULL entry in HeapReleaseEntry()", "");
    HeapValidateHdr(entry->block, "HeapReleaseEntry()");

    // verify removed from list and remove if necessary
    if (entry->next || entry->prev || entry->block->entry) {
        HeapRemoveFromList(entry);
    }

    // clear out the data
    entry->block->entry = 0;
    entry->block = 0;
    entry->size = 0;
}


//
// -- Create a new list entry for the hole
//    ------------------------------------
static OrderedList_t *HeapNewListEntry(KHeapHeader_t *hdr, bool add)
{
    int i;
    OrderedList_t *ret;
    extern OrderedList_t fixedList[ORDERED_LIST_STATIC];

    assert(hdr != NULL);

    // Assume the hdr to be good; entry does not pass test
    for (i = 0; i < ORDERED_LIST_STATIC; i ++) {
        if (!fixedList[i].block) {
            ret = &fixedList[i];
            ret->block = hdr;
            ret->size = hdr->size;
            ret->next = ret->prev = 0;
            hdr->entry = ret;

            if (add) HeapAddToList(ret);

            HeapValidateHdr(hdr, "Created HeapNewListEntry()");
            return ret;
        }
    }

    HeapError("Unable to allocate a free OrderedList entry", "");
    return 0;
}


//
// -- Align a block to a Page boundary
//
//    Split an entry to the first page boundary after allocating the header.  This will result in a free block on the
//    left of the page boundary.  This block may be small and if so will need to be added to the previous block
//    (which is allocated by definition) or at the beginning of the heap memory (special case).
//
//    +------------------------------------------------------------------+
//    |  The entry before splitting.  Split will occur at some location  |
//    |  within the entry.                                               |
//    +------------------------------------------------------------------+
//
//    One of 2 results will occur (as below):
//
//                     Page
//                   Boundary
//                       |
//                       |
//                       V
//    +------------------+-----------------------------------------------+
//    |  A small block   |  A brand new entry inserted into the          |
//    |  too small to    |  ordered list for the remaining free memory.  |
//    |  add as a hole.  |                                               |
//    +------------------+-----------------------------------------------+
//    |  A block of new  |  A brand new entry inserted into the          |
//    |  free memory     |  ordered list for the remaining free memory.  |
//    |  inserted to lst |                                               |
//    +------------------+-----------------------------------------------+
//
//    ---------------------------------------------------------------------------------------------------------------
static OrderedList_t *HeapAlignToPage(OrderedList_t *entry)
{
    KHeapHeader_t *newHdr, *oldHdr;
    KHeapFooter_t *newFtr, *oldFtr;
    size_t leftSize, rightSize;
    OrderedList_t *ret;

    if (!assert(entry != 0)) HeapError("NULL entry in HeapAlignToPage()", "");
    HeapValidateHdr(entry->block, "HeapAlignToPage()");

    // initialize the working variables
    oldHdr = entry->block;
    newHdr = (KHeapHeader_t *)(HeapCalcPageAdjustment(entry));
    newFtr = (KHeapFooter_t *)((char *)newHdr - sizeof(KHeapFooter_t));
    oldFtr = (KHeapFooter_t *)((char *)oldHdr + oldHdr->size - sizeof(KHeapFooter_t));
    leftSize = (char *)newFtr - (char *)oldHdr + sizeof(KHeapFooter_t);
    rightSize = (char *)oldFtr - (char *)newHdr + sizeof(KHeapFooter_t);

    HeapReleaseEntry(entry);            // will have better one(s) later

    // size the left block properly
    if (leftSize < MIN_HOLE_SIZE) {
        KHeapHeader_t *wrkHdr;

        wrkHdr = ((KHeapFooter_t *)((Byte_t *)oldHdr - sizeof(KHeapFooter_t )))->hdr;

        if ((Byte_t *)wrkHdr >= kHeap->strAddr) {
            KHeapFooter_t sav;
            KHeapFooter_t *tmp = (KHeapFooter_t *)((char *)wrkHdr + wrkHdr->size - sizeof(KHeapFooter_t));

            sav = *tmp;
            wrkHdr->size += leftSize;

            tmp = (KHeapFooter_t *)((char *)wrkHdr + wrkHdr->size - sizeof(KHeapFooter_t));
            *tmp = sav;
            HeapValidateHdr(wrkHdr, "Work Header in HeapAlignToPage()");
        }
        oldHdr = 0;
    } else {
        oldHdr->_magicUnion.magicHole = HEAP_MAGIC;
        oldHdr->_magicUnion.mhStruct.isHole = 1;
        oldHdr->size = leftSize;
        newFtr->hdr = oldHdr;
        newFtr->_magicUnion.magicHole = oldHdr->_magicUnion.magicHole;

        (void)HeapNewListEntry(oldHdr, 1);
        HeapValidateHdr(oldHdr, "Old Header in HeapAlignToPage() else");
    }

    // size the right block properly
    newHdr->_magicUnion.magicHole = HEAP_MAGIC;
    newHdr->_magicUnion.mhStruct.isHole = 1;
    newHdr->size = rightSize;
    oldFtr->hdr = newHdr;
    oldFtr->_magicUnion.magicHole = newHdr->_magicUnion.magicHole;

    ret = HeapNewListEntry(newHdr, 1);
    if (oldHdr) HeapValidateHdr(oldHdr, "Old Header in HeapAlignToPage() at return");
    HeapValidateHdr(newHdr, "New Header in HeapAlignToPage() at return");
    return ret;
}


//
// -- Merge this hole with the one on the left
//    ----------------------------------------
static OrderedList_t *HeapMergeLeft(KHeapHeader_t *hdr)
{
    KHeapFooter_t *leftFtr = NULL;
    KHeapHeader_t *leftHdr = NULL;
    KHeapFooter_t *thisFtr = NULL;

    if (!assert(hdr != NULL)) HeapError("Bad Header passed into HeapMergeLeft()", "");

    thisFtr = (KHeapFooter_t *)((char *)hdr + hdr->size - sizeof(KHeapFooter_t));
    leftFtr = (KHeapFooter_t *)((char *)hdr - sizeof(KHeapFooter_t));

    // -- Check of this fits before dereferencing the pointer -- may end in `#PF` if first block
    if ((Byte_t *)leftHdr < kHeap->strAddr) return 0;
    leftHdr = leftFtr->hdr;

    if (!leftHdr->_magicUnion.mhStruct.isHole) return 0;        // make sure the left block is a hole

    HeapReleaseEntry(leftHdr->entry);

    leftHdr->size += hdr->size;
    thisFtr->hdr = leftHdr;
    leftHdr->_magicUnion.mhStruct.isHole = thisFtr->_magicUnion.mhStruct.isHole = 1;

    return HeapNewListEntry(leftHdr, 0);
}


//
// -- Merge a new hole with the existing hols on the right side of this one in memory
//    -------------------------------------------------------------------------------
static OrderedList_t *HeapMergeRight(KHeapHeader_t *hdr)
{
    KHeapFooter_t *rightFtr;
    KHeapHeader_t *rightHdr;

    if (!assert(hdr != NULL)) HeapError("Bad Header passed into HeapMergeRight()", "");

    rightHdr = (KHeapHeader_t *)((Byte_t *)hdr + hdr->size);
    rightFtr = (KHeapFooter_t *)((Byte_t *)rightHdr + rightHdr->size - sizeof(KHeapFooter_t));

    if ((Byte_t *)rightFtr + sizeof(KHeapFooter_t) > kHeap->endAddr) return 0;
    HeapValidateHdr(rightHdr, "rightHeader in HeapMergeRight()");
    if (!rightHdr->_magicUnion.mhStruct.isHole) return 0;        // make sure the left block is a hole

    HeapReleaseEntry(rightHdr->entry);
    hdr->size += rightHdr->size;
    rightFtr->hdr = hdr;
    hdr->_magicUnion.mhStruct.isHole = rightFtr->_magicUnion.mhStruct.isHole = 1;

    return HeapNewListEntry(hdr, 0);
}


//
// -- Split a block to the indicated size
//    -----------------------------------
static KHeapHeader_t *HeapSplitAt(OrderedList_t *entry, size_t adjustToSize)
{
    KHeapHeader_t *newHdr, *oldHdr;
    KHeapFooter_t *newFtr, *oldFtr;
    size_t newSize;

    if (!assert(entry != NULL)) HeapError("NULL entry in HeapSplitAt()", "");
    HeapValidateHdr(entry->block, "HeapSplitAt()");
    HeapValidatePtr("HeapSplitAt()");

    // initialize the working variables
    oldHdr = entry->block;
    oldFtr = (KHeapFooter_t *)((Byte_t *)oldHdr + oldHdr->size - sizeof(KHeapFooter_t));
    newHdr = (KHeapHeader_t *)((Byte_t *)oldHdr + adjustToSize);
    newFtr = (KHeapFooter_t *)((Byte_t *)newHdr - sizeof(KHeapFooter_t));
    newSize = oldHdr->size - adjustToSize;

    HeapReleaseEntry(entry);        // release entry; will replace with back half

    // size the allocated block properly
    oldHdr->size = adjustToSize;
    oldHdr->_magicUnion.mhStruct.isHole = 0;
    newFtr->hdr = oldHdr;
    newFtr->_magicUnion.magicHole = oldHdr->_magicUnion.magicHole;

    // create the new hole and add it to the list
    newHdr->_magicUnion.magicHole = HEAP_MAGIC;
    newHdr->_magicUnion.mhStruct.isHole = 1;
    newHdr->size = newSize;
    oldFtr->_magicUnion.magicHole = newHdr->_magicUnion.magicHole;
    oldFtr->hdr = newHdr;

    (void)HeapNewListEntry(newHdr, 1);

    // make sure we didn't make a mess
    HeapValidateHdr(oldHdr, "HeapSplitAt [oldHdr]");
    HeapValidateHdr(newHdr, "HeapSplitAt [newHdr]");

    // return the header to the allocated block
    return oldHdr;
}


//
// -- Initialize the heap structures
//
//    Create and initialize the internal Century heap structures.
//
//    Please note that we are allocating a starting block of memory statically.  This block is called
//    `heapMemoryBlock`.  The loader will have allocated frames for it but this heap is not located in the right area
//    of virtual address.  So,  part of the responsibility of this initialization step is to unmap these from the
//    kernel binary and then remap them into the Heap virtual address space at 0xd0000000.  By doing this, the
//    kernel should be able to get enough heap operational to begin to send messages, and then add more then the
//    PMM is operational.
//
//    Now, since I am moving the heap from the end of the kernel (which is how this was originally written), to a
//    standalone block of virtual address space, there are some chages that will need to be made.  Fortunately, the
//    design allows for this change relatively easily.
//    ------------------------------
void HeapInit(void)
{
    if (kHeap && kHeap->initialized) return;

#if DEBUG_ENABLED(HeapInit)

    KernelPrintf("Start heap initialization\n");

#endif

    Addr_t vAddr = heapStart;
    Addr_t vLimit = vAddr + INITIAL_HEAP;

    for ( ; vAddr < vLimit; vAddr += 0x1000) {
#if DEBUG_ENABLED(HeapInit)

        KernelPrintf(".. mapping addr %p (limit %p)\n", vAddr, vLimit);

#endif
        MmuMapPage(vAddr, PmmAlloc(), PG_WRT);
    }

#if DEBUG_ENABLED(HeapInit)

    KernelPrintf(".. initial pages mapped\n");

#endif

    // -- Set up the heap structure and list of open blocks
    KHeapFooter_t *tmpFtr;

    kMemSetB(fixedList, 0, sizeof(fixedList));  // this line causes a problem

#if DEBUG_ENABLED(HeapInit)

    KernelPrintf(".. fixedList cleared\n");

#endif

    // -- Build the first free block which is all allocated
#if DEBUG_ENABLED(HeapInit)

    KernelPrintf(".. building the first block at address %p\n", heapStart);

#endif

    fixedList[0].block = (KHeapHeader_t *)heapStart;
    fixedList[0].next = 0;
    fixedList[0].prev = 0;
    fixedList[0].size = INITIAL_HEAP;

    _heap.strAddr = (Byte_t *)heapStart;
    _heap.endAddr = ((Byte_t *)_heap.strAddr) + fixedList[0].size;
    _heap.maxAddr = (Byte_t *)heapEnd;

    _heap.heapMemory = _heap.heap512 = _heap.heap1K =
            _heap.heap4K = _heap.heap16K = &fixedList[0];

#if DEBUG_ENABLED(HeapInit)

    KernelPrintf(".. initializing the first header at %p\n", fixedList[0].block);

#endif

    fixedList[0].block->_magicUnion.magicHole = HEAP_MAGIC;
    fixedList[0].block->_magicUnion.mhStruct.isHole = 1;
    fixedList[0].block->size = fixedList[0].size;
    fixedList[0].block->entry = &fixedList[0];

#if DEBUG_ENABLED(HeapInit)

    KernelPrintf(".. initializing the first footer\n");

#endif

    tmpFtr = (KHeapFooter_t *)(((char *)fixedList[0].block) +
            fixedList[0].size - sizeof(KHeapFooter_t));
    tmpFtr->_magicUnion.magicHole = fixedList[0].block->_magicUnion.magicHole;
    tmpFtr->hdr = fixedList[0].block;

    fixedListUsed = 1;
    kHeap = &_heap;

    kHeap->initialized = true;

#if DEBUG_ENABLED(HeapInit)

    KernelPrintf("Heap Created\n");
    KernelPrintf("  Heap Start Location: %p\n", kHeap->strAddr);
    KernelPrintf("  Current Heap Size..: %p\n", fixedList[0].size);
    KernelPrintf("  Heap End Location..: %p\n", kHeap->endAddr);

#endif
}


//
// -- Alloc a block of memory from the heap
//
//  Allocate a number of bytes from the heap, returning a pointer to the block of memory requested.  This block of
//  memory is adjusted for the number of bytes in the header block; the pointer is the first byte beyond the header.
//
//  The following conditions are accounted for in this function:
//  1.  A hole is found that is EXACTLY the size needed (rare) -- allocate it
//  2.  A hole is found that is slightly larger than needed, but not enough space to realistically leave another
//      hole behind -- allocate the hole
//  3.  A hole is found to be too big -- split the hole and allocate the correct amount of heap
//  4.  A hole that is not enough can be found -- return 0
//
//  When a request for memory must be page aligned:
//  5.  A hole before the allocated memory is too small -- add it to the previous block
//  6.  A hole after the allocated memory is too small -- allocate it with the requested memory
//  7.  Both the 2 situations above -- completed both actions
//  8.  A hole is too big -- split it accordingly taking into account the above
//
//  TODO: Fix potential memory leak when multiple small alignments get added to previous blocks that will never
//        be deallocated.
//    --------------------------------------------------------------------------------------------------------------
void *HeapAlloc(size_t size, bool align)
{
    Addr_t flags = DisableInt();
    SpinLock(&heapLock); {
        if (unlikely(!kHeap->initialized)) HeapInit();

        size_t adjustedSize;
        OrderedList_t *entry;
        KHeapHeader_t *hdr;

        if (size < HEAP_SMALLEST) size = HEAP_SMALLEST;            // must allocate at least 1 byte

        if (size & (BYTE_ALIGNMENT - 1)) {                        // Check for alignment
            size += BYTE_ALIGNMENT;
            size &= ~(BYTE_ALIGNMENT - 1);
        }

        adjustedSize = size + sizeof(KHeapHeader_t) + sizeof(KHeapFooter_t);

again:
        entry = HeapFindHole(adjustedSize, align);

        // -- are we out of memory?
        if (!entry) {
            HeapCheckHealth();
            if(HeapExpand()) goto again;

            SpinUnlock(&heapLock);
            RestoreInt(flags);

            return 0;
        }

        HeapValidateHdr(entry->block, "HeapAlloc()");
        hdr = entry->block;

        // if we are aligning, take care of it now
        if (align) {
            entry = HeapAlignToPage(entry);        // must reset entry

            if (!entry) {
                HeapCheckHealth();
                if(HeapExpand()) goto again;

                SpinUnlock(&heapLock);
                RestoreInt(flags);

                return 0;
            }

            HeapValidateHdr(entry->block, "HeapAlloc() after alignment");
            hdr = entry->block;
        }

        // perfect fit -OR- just a little too big
        if (hdr->size == adjustedSize || adjustedSize - hdr->size < MIN_HOLE_SIZE) {
            KHeapFooter_t *ftr;

            ftr = (KHeapFooter_t *)((Byte_t *)hdr + hdr->size - sizeof(KHeapFooter_t));

            HeapReleaseEntry(entry);
            hdr->_magicUnion.mhStruct.isHole = 0;
            ftr->_magicUnion.mhStruct.isHole = 0;
            HeapValidateHdr(hdr, "Resulting Header before return (good size)");
            HeapCheckHealth();

            SpinUnlock(&heapLock);
            RestoreInt(flags);

            return (void *)((Byte_t *)hdr + sizeof(KHeapHeader_t));
        }

        // the only thing left is that it is too big and needs to be split
        hdr = HeapSplitAt(entry, adjustedSize);        // var entry is no longer valid after call
        HeapValidatePtr("HeapAlloc()");
        HeapValidateHdr(hdr, "Resulting Header before return (big size)");
        HeapCheckHealth();

        SpinUnlock(&heapLock);
        RestoreInt(flags);

        return (void *)((Byte_t *)hdr + sizeof(KHeapHeader_t));
    }
}



//
// -- Free a block of memory back to the heap
//    ---------------------------------------
void HeapFree(void *mem)
{
    OrderedList_t *entry = 0;
    KHeapHeader_t *hdr;
    KHeapFooter_t *ftr;

    if (!mem) return;

    Addr_t flags = DisableInt();
    SpinLock(&heapLock); {
        if (unlikely(!kHeap->initialized)) HeapInit();

        hdr = (KHeapHeader_t *)((Byte_t *)mem - sizeof(KHeapHeader_t));
        ftr = (KHeapFooter_t *)((Byte_t *)hdr + hdr->size - sizeof(KHeapFooter_t));
        HeapValidateHdr(hdr, "Heap structures have been overrun by data!!");

        HeapCheckHealth();

        if (hdr->_magicUnion.mhStruct.isHole) goto exit;
        if (hdr->_magicUnion.magicHole != HEAP_MAGIC || ftr->_magicUnion.magicHole != HEAP_MAGIC) goto exit;
        if (ftr->hdr != hdr) goto exit;

        HeapCheckHealth();
        entry = HeapMergeRight(hdr);
        HeapCheckHealth();

        entry = HeapMergeLeft(hdr);
        HeapCheckHealth();
        if (entry) hdr = entry->block;        // reset header if changed

        if (!entry) entry = hdr->entry;        // if nothing changes, get this entry

        hdr->_magicUnion.mhStruct.isHole = ftr->_magicUnion.mhStruct.isHole = 1;
        if (entry) HeapAddToList(entry);    // now add to the ordered list
        else (void)HeapNewListEntry(hdr, 1);

    exit:
        HeapCheckHealth();

        SpinUnlock(&heapLock);
        RestoreInt(flags);
    }
}


