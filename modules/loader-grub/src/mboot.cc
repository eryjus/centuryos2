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


//#define USE_SERIAL

#include "types.h"
#include "mmu.h"
#include "mboot.h"
#include "serial.h"
#include "boot-interface.h"


extern BootInterface_t *kernelInterface;


//
// == MB1 Structures
//    --------------

//
// -- This is the loaded modules block (which will repeat)
//    ----------------------------------------------------
typedef struct Mb1Mods_t {
    uint32_t modStart;
    uint32_t modEnd;
    uint32_t modIdent;
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
// -- Memory Map
//    ----------
typedef struct Mb2MemMap_t {
    Mb2BasicTag_t tag;         // type == 6
    uint32_t entrySize;
    uint32_t entryVersion;
    struct {
        uint64_t baseAddr;
        uint64_t length;
        uint32_t type;
        uint32_t reserved;
    } entries [0];
} __attribute__((packed)) Mb2MemMap_t;


//
// -- When booted by MB1, get the kernel image location
//    -------------------------------------------------
static Addr_t MBootGetMb1Kernel(void)
{
    extern uint32_t mbData;
    uint64_t mbd = mbData;
    Addr_t rv = 0;

    MB1_t *mb1 = (MB1_t *)mbd;

    if (!mbData) return 0;

    SerialPutString("MB1\n");

    if (!MmuIsMapped(mbData)) {
        MmuMapPage(mbData, mbData >> 12, false);
    }


    // -- Handle module information (3)
    if (mb1->flags & (1<<3)) {
        uint64_t mAddr = mb1->modAddr;
        Mb1Mods_t *mods = (Mb1Mods_t *)mAddr;

        SerialPutString("Module loaded at Address ");
        SerialPutHex64(mAddr);
        SerialPutChar('\n');

        for (unsigned int i = 0; i < mb1->modCount; i ++) {
            char *name = (char *)((Addr_t)mods[i].modIdent);

            if (!MmuIsMapped((Addr_t)name)) {
                MmuMapPage((Addr_t)name, ((Addr_t)name) >> 12, false);
            }

            SerialPutString(".. ");
            SerialPutString(name);
            SerialPutString(" @ ");
            SerialPutHex64(mods[i].modStart);
            SerialPutChar('\n');

            if (name[0] == 'k' &&
                    name[1] == 'e' &&
                    name[2] == 'r' &&
                    name[3] == 'n' &&
                    name[4] == 'e' &&
                    name[5] == 'l') {
                rv = mods->modStart;
            } else {
                kernelInterface->modAddr[kernelInterface->modCount ++] = mods[i].modStart;
            }
        }
    }


    // -- Handle memory map inforation (6)
    if (mb1->flags & (1<<6)) {
        uint32_t size = mb1->mmapLen;
        int cnt = 0;
        Mb1MmapEntry_t *entry = (Mb1MmapEntry_t *)(((Addr_t)mb1->mmapAddr));
        while (size) {
            SerialPutString(".. Memory Block: ");
            SerialPutHex64(entry->mmapAddr);
            SerialPutString("; size: ");
            SerialPutHex64(entry->mmapLength);
            SerialPutString("; type: ");
            SerialPutHex32(entry->mmapType);
            SerialPutChar('\n');


            if (entry->mmapType == 1 && cnt < MAX_MEM) {
                kernelInterface->memBlocks[cnt].start = entry->mmapAddr;
                kernelInterface->memBlocks[cnt].end = entry->mmapAddr + entry->mmapLength;

                SerialPutString(".. (Checking start: ");
                SerialPutHex64(kernelInterface->memBlocks[cnt].start);
                SerialPutString("; end: ");
                SerialPutHex64(kernelInterface->memBlocks[cnt].end);
                SerialPutString(")\n");

                cnt ++;
            }

            size -= (entry->mmapSize + 4);
            entry = (Mb1MmapEntry_t *)(((Addr_t)entry) + entry->mmapSize + 4);
        }
    }


    return rv;
}


//
// -- When booted by MB2, get the kernel image location
//    -------------------------------------------------
static Addr_t MBootGetMb2Kernel(void)
{
    extern uint32_t mbData;
    uint32_t locn = (Addr_t)mbData + sizeof(Mb2Fixed_t);
    bool lastTag = false;
    Addr_t rv = 0;

    if (!mbData) return 0;

    SerialPutString("MB2\n");

    if (!MmuIsMapped(mbData)) {
        MmuMapPage(mbData, mbData >> 12, false);
    }

    while (!lastTag) {
        Mb2BasicTag_t *tag = (Mb2BasicTag_t *)(Addr_t)locn;

        switch (tag->type) {
        case 3: {            // -- modules
            Mb2Module_t *m = (Mb2Module_t *)(Addr_t)locn;

            if (m->name[0] == 'k' &&
                    m->name[1] == 'e' &&
                    m->name[2] == 'r' &&
                    m->name[3] == 'n' &&
                    m->name[4] == 'e' &&
                    m->name[5] == 'l') {
                rv = m->modStart;
            } else {
                kernelInterface->modAddr[kernelInterface->modCount ++] = m->modStart;
            }

            break;
        }


        case 6: {           // -- memory map
            Mb2MemMap_t *mmap = (Mb2MemMap_t *)(Addr_t)locn;
            uint32_t s = tag->size / mmap->entrySize;
            int cnt = 0;
            for (uint32_t i = 0; i < s; i ++) {
                if (mmap->entries[i].type == 1 && cnt < MAX_MEM) {
                    kernelInterface->memBlocks[cnt].start = mmap->entries[i].baseAddr;
                    kernelInterface->memBlocks[cnt].end = mmap->entries[i].length + mmap->entries[i].baseAddr;
                    cnt ++;
                }
            }

            break;
        }


        case 0:             // -- last block processed
            lastTag = true;
            break;
        }

        locn += (tag->size + (~(tag->size - 1) & 0x7));
    }


    return rv;
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

