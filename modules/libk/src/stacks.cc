//===================================================================================================================
//
//  stacks.cc -- Some helpers to managing stack spaces
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  There are several kernel stack locations that need to be managed.  These will all use the same address space.
//  These functions will assist in this.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-May-17  Initial  v0.0.9b  ADCL  Initial version
//
//===================================================================================================================



#include "types.h"
#include "kernel-funcs.h"
#include "heap.h"
#include "stacks.h"



//
// -- This is the stack Manager structure
//    -----------------------------------
typedef struct StackManager_t {
    Addr_t startStart;
    Addr_t stackSize;
    size_t stackCount;
    Bitmap_t stacks[0];
} StackManager_t;



//
// -- make sure we initialize the stack handlers on first call
//    --------------------------------------------------------
bool initialized = false;



//
// -- Lock for managing the stacks
//    ----------------------------
static Spinlock_t lock = {0};



//
// -- This is the stack manager
//    -------------------------
static StackManager_t *stackManager = NULL;



//
// -- Initialize the stack structures
//    -------------------------------
static void StackInit(void)
{
    extern Addr_t __stackStart;
    extern Addr_t __stackCount;
    extern Addr_t __stackSize;

    int bits = sizeof(Bitmap_t) * 8;
    int stacksCount = (__stackCount + (bits - 1)) / bits;

    stackManager = (StackManager_t *)HeapAlloc(sizeof(StackManager_t) + (stacksCount * sizeof(Bitmap_t)), false);
    if (!assert(stackManager != NULL)) return;

    stackManager->stackCount = __stackCount;
    stackManager->stackSize = __stackSize;
    stackManager->startStart = __stackStart;

    kMemSetB(stackManager->stacks, 0, stacksCount * sizeof(Bitmap_t));
    initialized = true;
}



//
// -- Do the actual Stack Allocation
//    ------------------------------
static void StackDoAlloc(Addr_t stack)
{
    if (unlikely(!initialized)) StackInit();

    stack &= ~(stackManager->stackSize - 1);

    int bits = sizeof(Bitmap_t) * 8;
    int idx = (stack - stackManager->startStart) / bits;
    int off = (stack - stackManager->startStart) % bits;

    if (!assert(stack >= stackManager->startStart)) return;
    if (!assert(stack < stackManager->startStart + (stackManager->stackCount * stackManager->stackSize))) return;

    stackManager->stacks[idx] |= (1 << off);
}



//
// -- Allocate a stack
//    ----------------
void StackAlloc(Addr_t stack)
{
    Addr_t flags = DisableInt();
    SpinLock(&lock);
    StackDoAlloc(stack);
    SpinUnlock(&lock);
    RestoreInt(flags);
}



//
// -- Find an available stack and allocate it
//    ---------------------------------------
Addr_t StackFind(void)
{
    Addr_t rv = 0;
    Addr_t flags = DisableInt();
    SpinLock(&lock); {
        if (unlikely(!initialized)) StackInit();

        for (int i = 0; i < stackManager->stackCount; i ++) {
            if (stackManager->stacks[i] != (Addr_t)-1) {
                for (int j = 0; j < 32; j ++) {
                    if ((stackManager->stacks[i] & (1 << j)) == 0) {
                        rv = stackManager->startStart + (stackManager->stackSize * ((i * 32) + j));
                        StackDoAlloc(rv);
                        KernelPrintf("In address space %p, allocating stack %p\n", GetAddressSpace(), rv);
                        goto exit;
                    }
                }
            }
        }

exit:
        SpinUnlock(&lock);
        RestoreInt(flags);
    }

    return rv;
}



//
// -- Release a stack
//    ---------------
void StackRelease(Addr_t stack)
{
    int bits = sizeof(Bitmap_t) * 8;
    int idx = (stack - stackManager->startStart) / bits;
    int off = (stack - stackManager->startStart) % bits;

    if (!assert(stack >= stackManager->startStart)) return;
    if (!assert(stack < stackManager->startStart + (stackManager->stackCount * stackManager->stackSize))) return;

    Addr_t flags = DisableInt();
    SpinLock(&lock); {
        stackManager->stacks[idx] &= ~(1 << off);
        SpinUnlock(&lock);
        RestoreInt(flags);
    }
}



