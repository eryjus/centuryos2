//===================================================================================================================
//
//  gdt.cc -- handle interfacing with the GDT
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-29  Initial  v0.0.4   ADCL  Initial version
//
//===================================================================================================================


//#define USE_SERIAL

#include "types.h"
#include "printf.h"
#include "tss.h"


//
// -- This is a descriptor used for the GDT and LDT
//    ---------------------------------------------
typedef union Descriptor_t {
    struct {
        unsigned long limitLow : 16;        // Low bits (15-0) of segment limit
        unsigned long baseLow : 16;         // Low bits (15-0) of segment base address
        unsigned long baseMid : 8;          // Middle bits (23-16) of segment base address
        unsigned long type : 4;             // Segment type (see GDT_* constants)
        unsigned long s : 1;                // 0 = system, 1 = application (1 for code/data)
        unsigned long dpl : 2;              // Descriptor Privilege Level
        unsigned long p : 1;                // Present (must be 1)
        unsigned long limitHi : 4;          // High bits (19-16) of segment limit
        unsigned long avl : 1;              // Unused (available for software use)
        unsigned long bit64 : 1;            // 1 = 64-bit segment
        unsigned long db : 1;               // 0 = 16-bit segment, 1 = 32-bit segment
        unsigned long g : 1;                // Granularity: limit scaled by 4K when set
        unsigned long baseHi : 8;           // High bits (31-24) of segment base address
    };
    struct {
        unsigned long baseHi32 : 32;        // the highest 32 bits of a 64-bit TSS ase address
        unsigned long zero : 32;            // reserved as 0
    } alt;
} Descriptor_t;


//
// -- A helper macro use to define the NULL Selector
//    ----------------------------------------------
#define NULL_GDT    { { 0 } }


//
// -- A helper macro used to define the kernel code
//    0x00 c f 9 a 00 0000 ffff
//    ---------------------------------------------
#define KCODE_GDT        { {            \
    .limitLow = 0,                      \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x0a,                       \
    .s = 1,                             \
    .dpl = 0,                           \
    .p = 1,                             \
    .limitHi = 0,                       \
    .avl = 0,                           \
    .bit64 = 1,                         \
    .db = 0,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
} }


//
// -- A helper macro used to define the kernel data
//    0x00 c f 9 2 00 0000 ffff
//    ---------------------------------------------
#define KDATA_GDT        { {            \
    .limitLow = 0,                      \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x02,                       \
    .s = 1,                             \
    .dpl = 0,                           \
    .p = 1,                             \
    .limitHi = 0,                       \
    .avl = 0,                           \
    .bit64 = 1,                         \
    .db = 0,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
} }


//
// -- A helper macro used to define the user code
//    0x00 c f f a 00 0000 ffff
//    -------------------------------------------
#define UCODE_GDT        { {            \
    .limitLow = 0,                      \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x0a,                       \
    .s = 1,                             \
    .dpl = 3,                           \
    .p = 1,                             \
    .limitHi = 0,                       \
    .avl = 0,                           \
    .bit64 = 1,                         \
    .db = 0,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
} }


//
// -- A helper macro used to define the user data
//    0x00 c f f 2 00 0000 ffff
//    -------------------------------------------
#define UDATA_GDT        { {            \
    .limitLow = 0,                      \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x02,                       \
    .s = 1,                             \
    .dpl = 3,                           \
    .p = 1,                             \
    .limitHi = 0,                       \
    .avl = 0,                           \
    .bit64 = 1,                         \
    .db = 0,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
} }


//
// -- A helper macro to define a segment selector specific to the per-cpu data for a given CPU.
//    -----------------------------------------------------------------------------------------
#define GS_GDT(locn)        { {         \
    .limitLow = 0x0f,                   \
    .baseLow = ((locn) & 0xffff),       \
    .baseMid = (((locn) >> 16) & 0xff), \
    .type = 0x02,                       \
    .s = 1,                             \
    .dpl = 3,                           \
    .p = 1,                             \
    .limitHi = 0,                       \
    .avl = 0,                           \
    .bit64 = 1,                         \
    .db = 1,                            \
    .g = 1,                             \
    .baseHi = (((locn) >> 24) & 0xff),  \
} }


//
// -- A helper macro used to define the loader code
//    0x00 c f 9 a 00 0000 ffff
//    ---------------------------------------------
#define LCODE_GDT        { {            \
    .limitLow = 0,                      \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x0a,                       \
    .s = 1,                             \
    .dpl = 0,                           \
    .p = 1,                             \
    .limitHi = 0,                       \
    .avl = 0,                           \
    .bit64 = 1,                         \
    .db = 0,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
} }


//
// -- A helper macro used to define the loader data
//    0x00 c f 9 2 00 0000 ffff
//    ---------------------------------------------
#define LDATA_GDT        { {            \
    .limitLow = 0,                      \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x02,                       \
    .s = 1,                             \
    .dpl = 0,                           \
    .p = 1,                             \
    .limitHi = 0,                       \
    .avl = 0,                           \
    .bit64 = 1,                         \
    .db = 0,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
} }


//
// -- A helper macro used to define the 64-bit TSS (lower entry)
//    ----------------------------------------------------------
#define TSSL32_GDT(locn)       { {                  \
    .limitLow = ((sizeof(Tss_t) - 1) & 0xffff),     \
    .baseLow = ((locn) & 0xffff),                   \
    .baseMid = (((locn) >> 16) & 0xff),             \
    .type = 0x9,                                    \
    .s = 0,                                         \
    .dpl = 0,                                       \
    .p = 1,                                         \
    .limitHi = (((sizeof(Tss_t) - 1) >> 16) & 0xf), \
    .avl = 0,                                       \
    .bit64 = 0,                                     \
    .db = 0,                                        \
    .g = 0,                                         \
    .baseHi = (((locn) >> 24) & 0xff),              \
} }


//
// -- A helper macro used to define the 64-bit TSS (upper entry)
//    ----------------------------------------------------------
#define TSSU32_GDT(locn)       { .alt = {           \
    .baseHi32 = (uint32_t)(locn >> 32),             \
    .zero = 0,                                      \
} }


//
// -- This is the statically defined GDT, which will be used for everything
//    ---------------------------------------------------------------------
Descriptor_t __attribute__((aligned(16))) gdt[9 + (3 * MAX_CPU)] = {
    NULL_GDT,                                           // 0x00

    // -- common segment selectors
    KCODE_GDT,                                          //  0x08
    KDATA_GDT,                                          //  0x10
    UCODE_GDT,                                          //  0x18
    UDATA_GDT,                                          //  0x20
    KDATA_GDT,                                          //  0x28
    UDATA_GDT,                                          //  0x30
    LCODE_GDT,                                          //  0x38
    LDATA_GDT,                                          //  0x40

    // -- CPU0
    GS_GDT(0),                                          //  0x48
    TSSL32_GDT((Addr_t)&tss[0]),                        //  0x50
    TSSU32_GDT((Addr_t)&tss[0]),                        // (0x58)

    // -- CPU1
    GS_GDT(0),                                          //  0x60
    TSSL32_GDT((Addr_t)&tss[1]),                        //  0x68
    TSSU32_GDT((Addr_t)&tss[1]),                        // (0x70)

    // -- CPU2
    GS_GDT(0),                                          //  0x78
    TSSL32_GDT((Addr_t)&tss[2]),                        //  0x80
    TSSU32_GDT((Addr_t)&tss[2]),                        // (0x88)

    // -- CPU3
    GS_GDT(0),                                          //  0x90
    TSSL32_GDT((Addr_t)&tss[3]),                        //  0x98
    TSSU32_GDT((Addr_t)&tss[3]),                        // (0xa0)
};


//
// -- The typedef and the variable declaration must be separate to keep clang from complaining
//    ----------------------------------------------------------------------------------------
typedef struct Gdtr_t {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) Gdtr_t;


// -- not an implied static variable
Gdtr_t gdtr = {
    .limit = (sizeof(gdt) - 1),
    .base = (uint64_t)gdt,
};



//
// -- Initialize the GS GDT entry
//    ---------------------------
extern "C" void GsInit(void);
void GsInit(void)
{
    cpus[0].cpuNum = 0;
    cpus[0].state = CPU_STARTED;

    kprintf("Initializing GS to be at base %p\n", &(cpus[0].cpu));
    WRMSR(IA32_KERNEL_GS_BASE, (Addr_t)&(cpus[0].cpu));
    __asm volatile ("swapgs" ::: "memory");
}


