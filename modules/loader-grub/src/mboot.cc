//===================================================================================================================
//
//  mboot.cc -- Multiboot parsing functions
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-03  Initial  v0.0.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "mmu.h"
#include "mboot.h"
#include "serial.h"


//
// == MB1 Structures
//    --------------

//
// -- This is the loaded modules block (which will repeat)
//    ----------------------------------------------------
typedef struct Mb1Mods_t {
    uint32_t modStart;
    uint32_t modEnd;
    char *modIdent;
    uint32_t modReserved;
} __attribute__((packed)) Mb1Mods_t;


//
// -- Memory Map entries, which will repeat (pointer points to mmapAddr)
//    ------------------------------------------------------------------
typedef struct Mb1MmapEntry_t {
    uint32_t mmapSize;
    uint64_t mmapAddr;
    uint64_t mmapLength;
    uint32_t mmapType;
} __attribute__((packed)) Mb1MmapEntry_t;


//
// -- This is the Multiboot 1 information structure as defined by the spec
//    --------------------------------------------------------------------
typedef struct MB1_t {
    //
    // -- These flags indicate which data elements have valid data
    //    --------------------------------------------------------
    const uint32_t flags;

    //
    // -- The basic memory limits are valid when flag 0 is set; these values are in kilobytes
    //    -----------------------------------------------------------------------------------
    const uint32_t availLowerMem;
    const uint32_t availUpperMem;

    //
    // -- The boot device when flag 1 is set
    //    ----------------------------------
    const uint32_t bootDev;

    //
    // -- The command line for this kernel when flag 2 is set
    //    ---------------------------------------------------
    const uint32_t cmdLine;

    //
    // -- The loaded module list when flag 3 is set
    //    -----------------------------------------
    const uint32_t modCount;
    const uint32_t modAddr;

    //
    // -- The ELF symbol information (a.out-type symbols are not supported) when flag 5 is set
    //    ------------------------------------------------------------------------------------
    const uint32_t shdrNum;                 // may still be 0 if not available
    const uint32_t shdrSize;
    const uint32_t shdrAddr;
    const uint32_t shdrShndx;

    //
    // -- The Memory Map information when flag 6 is set
    //    ---------------------------------------------
    const uint32_t mmapLen;
    const uint32_t mmapAddr;

    //
    // -- The Drives information when flag 7 is set
    //    -----------------------------------------
    const uint32_t drivesLen;
    const uint32_t drivesAddr;

    //
    // -- The Config table when flag 8 is set
    //    -----------------------------------
    const uint32_t configTable;

    //
    // -- The boot loader name when flag 9 is set
    //    ---------------------------------------
    const uint32_t bootLoaderName;

    //
    // -- The APM table location when bit 10 is set
    //    -----------------------------------------
    const uint32_t apmTable;

    //
    // -- The VBE interface information when bit 11 is set
    //    ------------------------------------------------
    const uint32_t vbeControlInfo;
    const uint32_t vbeModeInfo;
    const uint16_t vbeMode;
    const uint16_t vbeInterfaceSeg;
    const uint16_t vbeInterfaceOff;
    const uint16_t vbeInterfaceLen;

    //
    // -- The FrameBuffer information when bit 12 is set
    //    ----------------------------------------------
    const uint64_t framebufferAddr;
    const uint32_t framebufferPitch;
    const uint32_t framebufferWidth;
    const uint32_t framebufferHeight;
    const uint8_t framebufferBpp;
    const uint8_t framebufferType;
    union {
        struct {
            const uint8_t framebufferRedFieldPos;
            const uint8_t framebufferRedMaskSize;
            const uint8_t framebufferGreenFieldPos;
            const uint8_t framebufferGreenMaskSize;
            const uint8_t framebufferBlueFieldPos;
            const uint8_t framebufferBlueMaskSize;
        };
        struct {
            const uint32_t framebufferPalletAddr;
            const uint16_t framebufferPalletNumColors;
        };
    };
} __attribute__((packed)) MB1_t;


//
// == MB2 Structures
//    ==============

//
// -- The fixed multiboot info structure elements
//    -------------------------------------------
typedef struct Mb2Fixed_t {
    uint32_t totalSize;
    uint32_t reserved;
} __attribute__((packed)) Mb2Fixed_t;


//
// -- This is the basic tag header information -- every tag has one
//    -------------------------------------------------------------
typedef struct Mb2BasicTag_t {
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) Mb2BasicTag_t;


//
// -- A laoded module
//    ---------------
typedef struct Mb2Module_t {
    Mb2BasicTag_t tag;         // type == 3
    uint32_t modStart;
    uint32_t modEnd;
    char name[0];
} __attribute__((packed)) Mb2Module_t;


//
// -- When booted by MB1, get the kernel image location
//    -------------------------------------------------
static Addr_t MBootGetMb1Kernel(void)
{
    extern uint32_t mbData;
    uint64_t mbd = mbData;

    MB1_t *mb1 = (MB1_t *)mbd;

    if (!mbData) return 0;

    SerialPutString("MB1\n");

    if (!MmuIsMapped(mbData)) {
        MmuMapPage(mbData, mbData >> 12, false);
    }

    if (mb1->flags & (1<<3)) {
        uint64_t mAddr = mb1->modAddr;
        Mb1Mods_t *mods = (Mb1Mods_t *)mAddr;

        for (unsigned int i = 0; i < mb1->modCount; i ++) {
            char *name = mods[i].modIdent;

            if (!MmuIsMapped((Addr_t)name)) {
                MmuMapPage((Addr_t)name, ((Addr_t)name) >> 12, false);
            }

            if (name[0] != 'k') continue;
            if (name[1] != 'e') continue;
            if (name[2] != 'r') continue;
            if (name[3] != 'n') continue;
            if (name[4] != 'e') continue;
            if (name[5] != 'l') continue;

            return mods->modStart;
        }
    }

    return 0;
}


//
// -- When booted by MB2, get the kernel image location
//    -------------------------------------------------
static Addr_t MBootGetMb2Kernel(void)
{
    extern uint32_t mbData;
    uint32_t locn = (Addr_t)mbData + sizeof(Mb2Fixed_t);
    bool lastTag = false;

    if (!mbData) return 0;

    SerialPutString("MB2\n");

    if (!MmuIsMapped(mbData)) {
        MmuMapPage(mbData, mbData >> 12, false);
    }

    while (!lastTag) {
        Mb2BasicTag_t *tag = (Mb2BasicTag_t *)(Addr_t)locn;

        if (tag->type == 3) {
            Mb2Module_t *m = (Mb2Module_t *)(Addr_t)locn;

            if (m->name[0] != 'k') goto next;
            if (m->name[1] != 'e') goto next;
            if (m->name[2] != 'r') goto next;
            if (m->name[3] != 'n') goto next;
            if (m->name[4] != 'e') goto next;
            if (m->name[5] != 'l') goto next;

            return m->modStart;
        } else if (tag->type == 0) {
            lastTag = true;
        }

next:
        locn += (tag->size + (~(tag->size - 1) & 0x7));
    }


    return 0;
}


//
// -- Get the Kernel Location
//    -----------------------
Addr_t MBootGetKernel(void)
{
    extern uint32_t mbSig;

    SerialPutString("Checking MB Sig ");
    SerialPutHex32(mbSig);
    SerialPutChar('\n');

    if (mbSig == MB1SIG) return MBootGetMb1Kernel();
    if (mbSig == MB2SIG) return MBootGetMb2Kernel();
    return 0;
}

