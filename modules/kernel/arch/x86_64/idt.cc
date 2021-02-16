//===================================================================================================================
//
//  idt.cc -- handle interfacing with the IDT
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-13  Initial  v0.0.2   ADCL  Initial version
//
//===================================================================================================================


#ifndef USE_SERIAL
#define USE_SERIAL
#endif

#include "types.h"
#include "printf.h"
#include "serial.h"
#include "internal.h"
#include "idt.h"


#if __has_include("tss.h")
#include "tss.h"

Tss_t tss[MAX_CPU] = { { 0 } };

#else

#define TssInit()

#endif




//
// -- Prototypes
//    ----------
extern IdtHandlerFunc_t IdtGenericEntry;
extern IdtHandlerFunc_t IdtGenericEntryNoErr;

extern "C" {
    void IdtGenericHandler(Addr_t *);
}



//
// -- This is the structure of an IDT Entry
//    -------------------------------------
typedef struct IdtDescriptor_t {
    unsigned int offsetLo : 16;
    unsigned int segmentSel : 16;
    unsigned int ist : 3;
    unsigned int zero : 5;
    unsigned int type : 4;
    unsigned int off : 1;
    unsigned int dpl : 2;
    unsigned int p : 1;
    unsigned int offsetMid : 16;
    unsigned int offsetHi : 32;
    unsigned int unused : 32;
} IdtDescriptor_t;


//
// -- The IDT Table
//    -------------
IdtDescriptor_t idtTable[256] __attribute__((aligned(4096))) = { {0} };


//
// -- This is the IDT Register data
//    -----------------------------
struct {
    uint16_t limit;
    Addr_t loc;
} __attribute__((packed)) idtr;



//
// -- This is the generic interrupt handler
//    -------------------------------------
void __attribute__((noreturn)) IdtGenericHandler(Addr_t *stack)
{
    enum {
        GS  = 1,
        FS  = 2,
        ES  = 3,
        DS  = 4,
        CR3 = 5,
        R15 = 6,
        R14 = 7,
        R13 = 8,
        R12 = 9,
        R11 = 10,
        R10 = 11,
        R9  = 12,
        R8  = 13,
        RDI = 14,
        RSI = 15,
        RBP = 16,
        RDX = 17,
        RCX = 18,
        RBX = 19,
        RAX = 20,
        ERR = 21,
        RIP = 22,
        CS  = 23,
        RFLAGS = 24,
        RSP = 25,
        SS  = 26,
    };

    kprintf("An unknown error has occurred (Error Code %p)\n", stack[ERR]);
    kprintf("  RAX: %p\tR8 : %p\n", stack[RAX], stack[R8]);
    kprintf("  RBX: %p\tR9 : %p\n", stack[RBX], stack[R9]);
    kprintf("  RCX: %p\tR10: %p\n", stack[RCX], stack[R10]);
    kprintf("  RDX: %p\tR11: %p\n", stack[RDX], stack[R11]);
    kprintf("  RBP: %p\tR12: %p\n", stack[RBP], stack[R12]);
    kprintf("  RSI: %p\tR13: %p\n", stack[RSI], stack[R13]);
    kprintf("  RDI: %p\tR14: %p\n", stack[RDI], stack[R14]);
    kprintf("\t\t\t\tR15: %p\n", stack[R15]);
    kprintf("   SS: %x\tRSP: %p\n", stack[SS], stack[RSP]);
    kprintf("   CS: %x\tRIP: %p\n", stack[CS], stack[RIP]);
    kprintf("   DS: %x\t ES: %x\n", stack[DS], stack[ES]);
    kprintf("   FS: %x\t GS: %x\n", stack[FS], stack[GS]);

    while (true) {}
}



//
// -- Install the IDT
//    ---------------
void IdtInstall(void)
{
    for (int i = 0; i < 256; i ++) {
        switch (i) {
        case 8:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            IdtSetHandler(i, 8, &IdtGenericEntry, 0, 0);
            break;

        default:
            IdtSetHandler(i, 8, &IdtGenericEntryNoErr, 0, 0);
            break;
        }
    }

    idtr.loc = (Addr_t)idtTable;
    idtr.limit = sizeof(idtTable) - 1;

    __asm__("lidt %0" : : "m"(idtr));

    IdtSetHandler(0xe0, 8, &InternalTarget, 0, 0);

    TssInit();
}



//
// -- Get the address of an existing IDT Handler
//    ------------------------------------------
Addr_t IdtGetHandler(int i)
{
    IdtDescriptor_t desc = idtTable[i];
    Addr_t rv = 0;

    rv |= ((Addr_t)desc.offsetHi << 32);
    rv |= ((Addr_t)desc.offsetMid << 16);
    rv |= ((Addr_t)desc.offsetLo);

    return rv;
}



//
// - Set the address of a new IDT Handler
//   ------------------------------------
void IdtSetHandler(int i, uint16_t sec, IdtHandlerFunc_t *handler, int ist, int dpl)
{
    if (!handler) {
        IdtSetHandler(i, sec, &IdtGenericEntry, ist, dpl);
        return;
    }

    IdtDescriptor_t desc;

    desc.offsetLo = ((Addr_t)handler) & 0xffff;
    desc.segmentSel = sec;
    desc.ist = ist;
    desc.zero = 0;
    desc.type = 0xe;
    desc.off = 0;
    desc.dpl = dpl;
    desc.p = 1;
    desc.offsetMid = (((Addr_t)handler) >> 16) & 0xffff;
    desc.offsetHi = (((Addr_t)handler) >> 32) & 0xffffffff;
    desc.unused = 0;

    idtTable[i] = desc;
}



