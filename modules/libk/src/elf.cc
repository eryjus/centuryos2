//===================================================================================================================
//
//  elf.cc -- handle the loading of an elf file
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-04  Initial  v0.0.1   ADCL  Initial version
//
//===================================================================================================================


//#define USE_SERIAL


#include "types.h"
#include "mmu.h"
#include "serial.h"
#include "elf.h"


//
// -- No need to duplicate a function -- just call the MmuGetTable() function since it is exactly the same thing
//    ----------------------------------------------------------------------------------------------------------
static inline Frame_t PmmGetFrame(void) { return MmuGetTable(); }


//
// -- The number of identifying bytes
//    -------------------------------
#define ELF_NIDENT      16



//
// -- The following are the possible values for the ELF class, indicating what size the file objects
//    ----------------------------------------------------------------------------------------------
enum {
    ELFCLASS_NONE       = 0,    // Invalid
    ELFCLASS_32         = 1,    // 32-bit objects
    ELFCLASS_64         = 2,    // 64-bit objects
};



//
// -- The following are the possible values for the ELF Data encoding (big- or little-endian)
//    ---------------------------------------------------------------------------------------
enum {
    ELFDATA_NONE        = 0,    // Invalid
    ELFDATA_LSB         = 1,    // Binary values are in little endian order
    ELFDATA_MSB         = 2,    // Binary values are in big endian order
};



//
// -- these are the program header types
//    ----------------------------------
enum {
    PT_NULL = 0,
    PT_LOAD = 1,
    PT_DYNAMIC = 2,
    PT_INTERP = 3,
    PT_NOTE = 4,
    PT_SHLIB = 5,
    PT_PHDR = 6,
    PT_LOPROC = 0x70000000,
    PT_HIPROC = 0x7fffffff,
};



//
// -- The following are the defined types
//    -----------------------------------
enum {
    ET_NONE             = 0,    // No file type
    ET_REL              = 1,    // Relocatable file
    ET_EXEC             = 2,    // Executable file
    ET_DYN              = 3,    // Dynamic or Shared object file
    ET_CORE             = 4,    // Core file
    ET_LOOS             = 0xfe00, // Environment-specific use
    ET_HIOS             = 0xfeff,
    ET_LOPROC           = 0xff00, // Processor-specific use
    ET_HIPROC           = 0xffff,
};



//
// -- These are the program segment flags
//    -----------------------------------
enum {
    PF_X                = 0x01,     // The segment is executable
    PF_W                = 0x02,     // The segment is writable
    PF_R                = 0x04,     // The segment is readable
};



//
// -- Define the types that will be used by the ELF loader
//    ----------------------------------------------------
typedef uint64_t elf64Addr_t;
typedef uint64_t elf64Off_t;

typedef uint32_t elf32Addr_t;
typedef uint32_t elf32Off_t;

typedef int64_t elfSXWord_t;
typedef uint64_t elfXWord_t;
typedef int32_t elfSWord_t;
typedef uint32_t elfWord_t;
typedef uint16_t elfHalf_t;



//
// -- This is the 64-bit ELF File Header Definition
//    ---------------------------------------------
typedef struct Elf64EHdr_t {
    unsigned char eIdent[ELF_NIDENT];
    elfHalf_t eType;
    elfHalf_t eMachine;
    elfWord_t eversion;
    elf64Addr_t eEntry;
    elf64Off_t ePhOff;          // Program Header offset
    elf64Off_t eShOff;          // Section Header offset
    elfWord_t eFlags;
    elfHalf_t eHSize;           // Program Header Size
    elfHalf_t ePhEntSize;       // Program Header Entry Size
    elfHalf_t ePhNum;           // Program Header Entry Count
    elfHalf_t eShEntSize;       // Section Header Entry Size
    elfHalf_t eShNum;           // Section Header Entry Count
    elfHalf_t eShStrNdx;        // Section Number for the string table
} __attribute__((packed)) Elf64EHdr_t;



//
// -- This is the 64-bit ELF Program Header, which is needed to determine how to load the executable
//    ----------------------------------------------------------------------------------------------
typedef struct Elf64PHdr_t {
    elfWord_t pType;            // Type of segment
    elfWord_t pFlags;           // Segment Attributes
    elf64Off_t pOffset;         // Offset in file
    elf64Addr_t pVAddr;         // Virtual Address in Memory
    elf64Addr_t pPAddr;         // Reserved or meaningless
    elfXWord_t pFileSz;         // Size of segment in file
    elfXWord_t pMemSz;          // Size of segment in memory
    elfXWord_t pAlign;          // Alignment of segment
} __attribute__((packed)) Elf64PHdr_t;



//
// -- Evaluate an Elf Header and make sure it is loadable
//    ---------------------------------------------------
static bool ElfValidateHeader(Addr_t location)
{
    // -- this page must be mapped before calling this function
    Elf64EHdr_t *eHdr = (Elf64EHdr_t *)location;

    if (eHdr->eIdent[0] != 0x7f) return false;
    if (eHdr->eIdent[1] != 'E') return false;
    if (eHdr->eIdent[2] != 'L') return false;
    if (eHdr->eIdent[3] != 'F') return false;
    if (eHdr->eIdent[4] != ELFCLASS_64) return false;
    if (eHdr->eIdent[5] != ELFDATA_LSB) return false;
    if (eHdr->eType != ET_EXEC) return false;

    return true;
}


//
// -- Load the kernel address
//    -----------------------
Addr_t ElfLoadImage(Addr_t location)
{
    extern Frame_t earlyFrame;
    Addr_t rv = 0;

    SerialPutString("Parsing ELF\n");

    MmuMapPage(location, location >> 12, false);

    if (ElfValidateHeader(location)) {
        Elf64EHdr_t *eHdr = (Elf64EHdr_t *)location;
        Elf64PHdr_t *pHdr = (Elf64PHdr_t *)(location + eHdr->ePhOff);

        for (unsigned int i = 0; i < eHdr->ePhNum; i ++) {
            if (pHdr[i].pType == PT_LOAD) {
                Addr_t phys = location + pHdr[i].pOffset;
                Addr_t virt = pHdr[i].pVAddr;
                int64_t fSize = pHdr[i].pFileSz;
                int64_t mSize = pHdr[i].pMemSz;

                SerialPutString(".. type = ");
                SerialPutHex32(pHdr[i].pType);
                SerialPutChar('\n');

                while (mSize >= 0) {
                    SerialPutString(".. mapping elf; mSize = ");
                    SerialPutHex64(mSize);
                    SerialPutChar('\n');

                    Frame_t f = phys >> 12;

                    if (fSize <= 0) {
                        f = PmmGetFrame();
                    }

                    MmuMapPage(virt, f, pHdr[i].pType & PF_W);

                    virt += 4096;
                    phys += 4096;
                    mSize -= 4096;
                    fSize -= 4096;
                }
            }
        }

        SerialPutString("Loaded...\n");
        rv = eHdr->eEntry;
        MmuUnmapPage(location);
    }

    SerialPutString("Returning entry point ");
    SerialPutHex64(rv);
    SerialPutChar('\n');

    return rv;
}


