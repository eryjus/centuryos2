/****************************************************************************************************************//**
*   @file               elf.cc
*   @brief              Handle the loading of an ELF file
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Jan-04
*   @since              v0.0.01
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*   For the loader only, handle extracting and loading an elf file, preparing it for execution.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Jan-04 | Initial |  v0.0.01 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "mmu.h"
#include "serial.h"
#include "elf.h"


// -- if we are not compiling the loader, include the kernel functions
#ifndef __LOADER__
#include "kernel-funcs.h"
#endif



/****************************************************************************************************************//**
*   @def                ELF_NIDENT
*   @brief              The number of identifying bytes
*///-----------------------------------------------------------------------------------------------------------------
#define ELF_NIDENT      16




/****************************************************************************************************************//**
*   @enum               ElfClass_t
*   @brief              The classes of ELF files that be loaded
*///-----------------------------------------------------------------------------------------------------------------
enum ElfClass_t {
    ELFCLASS_NONE       = 0,        //!< Invalid
    ELFCLASS_32         = 1,        //!< 32-bit objects
    ELFCLASS_64         = 2,        //!< 64-bit objects
};



/****************************************************************************************************************//**
*   @enum               ElfData_t
*   @brief              The possible values for the ELF Data encoding (big- or little-endian)
*///-----------------------------------------------------------------------------------------------------------------
enum ElfData_t {
    ELFDATA_NONE        = 0,        //!< Invalid
    ELFDATA_LSB         = 1,        //!< Binary values are in little endian order
    ELFDATA_MSB         = 2,        //!< Binary values are in big endian order
};


/****************************************************************************************************************//**
*   @enum               PgmHdrType_t
*   @brief              Program header types
*///-----------------------------------------------------------------------------------------------------------------
enum PgmHdrType_t {
    PT_NULL             = 0,        //!< NULL type; do nothing
    PT_LOAD             = 1,        //!< Load the program section
    PT_DYNAMIC          = 2,        //!< Contains dynamic linking information
    PT_INTERP           = 3,        //!< Contains path name to interpreter
    PT_NOTE             = 4,        //!< Auxiliary information
    PT_SHLIB            = 5,        //!< Unspecified \note Programs that contain this section are not ELF compliant
    PT_PHDR             = 6,        //!< The program header table
    PT_LOPROC           = 0x70000000,//!< Processor specific semantics (\ref PT_LOPROC through \ref PT_HIPROC)
    PT_HIPROC           = 0x7fffffff,//!< Processor specific semantics (\ref PT_LOPROC through \ref PT_HIPROC)
};



/****************************************************************************************************************//**
*   @enum               FileType_t
*   @brief              File types
*///-----------------------------------------------------------------------------------------------------------------
enum FileType_t {
    ET_NONE             = 0,        //!< No file type
    ET_REL              = 1,        //!< Relocatable file
    ET_EXEC             = 2,        //!< Executable file
    ET_DYN              = 3,        //!< Dynamic or Shared object file
    ET_CORE             = 4,        //!< Core file
    ET_LOOS             = 0xfe00,   //!< Environment-specific use (\ref ET_LOOS through \ref ET_HIOS)
    ET_HIOS             = 0xfeff,   //!< Environment-specific use (\ref ET_LOOS through \ref ET_HIOS)
    ET_LOPROC           = 0xff00,   //!< Processor-specific use (\ref ET_LOPROC through \ref ET_HIPROC)
    ET_HIPROC           = 0xffff,   //!< Processor-specific use (\ref ET_LOPROC through \ref ET_HIPROC)
};



/****************************************************************************************************************//**
*   @enum               SegFlag_t
*   @brief              Program Segment Flags
*///-----------------------------------------------------------------------------------------------------------------
enum SegFlag_t {
    PF_X                = 0x01,     //!< The segment is executable
    PF_W                = 0x02,     //!< The segment is writable
    PF_R                = 0x04,     //!< The segment is readable
};



/****************************************************************************************************************//**
*   @typedef            elf64Addr_t
*   @brief              A 64-bit address value
*///-----------------------------------------------------------------------------------------------------------------
typedef uint64_t elf64Addr_t;



/****************************************************************************************************************//**
*   @typedef            elf64Off_t
*   @brief              A 64-bit offset value
*///-----------------------------------------------------------------------------------------------------------------
typedef uint64_t elf64Off_t;



/****************************************************************************************************************//**
*   @typedef            elf32Addr_t
*   @brief              A 32-bit address value
*///-----------------------------------------------------------------------------------------------------------------
typedef uint32_t elf32Addr_t;



/****************************************************************************************************************//**
*   @typedef            elf32Off_t
*   @brief              A 32-bit offset value
*///-----------------------------------------------------------------------------------------------------------------
typedef uint32_t elf32Off_t;



/****************************************************************************************************************//**
*   @typedef            elfSXWord_t
*   @brief              A 64-bit signed word
*///-----------------------------------------------------------------------------------------------------------------
typedef int64_t elfSXWord_t;



/****************************************************************************************************************//**
*   @typedef            elfXWord_t
*   @brief              A 64-bit unsigned word
*///-----------------------------------------------------------------------------------------------------------------
typedef uint64_t elfXWord_t;



/****************************************************************************************************************//**
*   @typedef            elfSWord_t
*   @brief              A 32-bit signed word
*///-----------------------------------------------------------------------------------------------------------------
typedef int32_t elfSWord_t;



/****************************************************************************************************************//**
*   @typedef            elfWord_t
*   @brief              A 32-bit unsigned word
*///-----------------------------------------------------------------------------------------------------------------
typedef uint32_t elfWord_t;



/****************************************************************************************************************//**
*   @typedef            elfHalf_t
*   @brief              A 16-bit unsigned half word
*///-----------------------------------------------------------------------------------------------------------------
typedef uint16_t elfHalf_t;



/****************************************************************************************************************//**
*   @typedef            Elf64EHdr_t
*   @brief              The 64-bit ELF header page
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Elf64EHdr_t
*   @brief              The 64-bit ELF header page
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Elf64EHdr_t {
    unsigned char eIdent[ELF_NIDENT];//!< A proper ELF identifier
    elfHalf_t eType;                //!< Object File Type
    elfHalf_t eMachine;             //!< Required Architecture
    elfWord_t eversion;             //!< Object file version \note Only version 1 is valid
    elf64Addr_t eEntry;             //!< Defined entry point
    elf64Off_t ePhOff;              //!< Program Header offset
    elf64Off_t eShOff;              //!< Section Header offset
    elfWord_t eFlags;               //!< Processor-specific flags
    elfHalf_t eHSize;               //!< Program Header Size
    elfHalf_t ePhEntSize;           //!< Program Header Entry Size
    elfHalf_t ePhNum;               //!< Program Header Entry Count
    elfHalf_t eShEntSize;           //!< Section Header Entry Size
    elfHalf_t eShNum;               //!< Section Header Entry Count
    elfHalf_t eShStrNdx;            //!< Section Number for the string table
} __attribute__((packed)) Elf64EHdr_t;



/****************************************************************************************************************//**
*   @typedef            Elf64PHdr_t
*   @brief              Formalization of the 64-bit ELF Program Header
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Elf64PHdr_t
*   @brief              The 64-bit ELF Program Header, which is needed to determine how to load the executable
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Elf64PHdr_t {
    elfWord_t pType;                //!< Type of segment
    elfWord_t pFlags;               //!< Segment Attributes
    elf64Off_t pOffset;             //!< Offset in file
    elf64Addr_t pVAddr;             //!< Virtual Address in Memory
    elf64Addr_t pPAddr;             //!< Reserved or meaningless
    elfXWord_t pFileSz;             //!< Size of segment in file
    elfXWord_t pMemSz;              //!< Size of segment in memory
    elfXWord_t pAlign;              //!< Alignment of segment
} __attribute__((packed)) Elf64PHdr_t;



/****************************************************************************************************************//**
*   @fn                 static bool ElfValidateHeader(Addr_t location)
*   @brief              Evaluate an Elf Header and make sure it is loadable
*
*   Evaluate the ELF header to see if it conforms to the requirements to be loaded.
*
*   @param              location        The address of the ELF header to evaluate
*
*   @returns            Whether there is a valid ELF header at the specified location
*
*   @retval             true            The ELF header is valid
*   @retval             false           The ELF header is not valid
*
*   @note               The page containing the ELF header most be mapped prior to calling this function.
*///-----------------------------------------------------------------------------------------------------------------
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



/********************************************************************************************************************
*   Documented in `elf.h`
*///-----------------------------------------------------------------------------------------------------------------
Addr_t ElfLoadImage(Addr_t location)
{
    Addr_t rv = 0;

    MmuMapPage(location, location >> 12, PG_NONE);

    if (ElfValidateHeader(location)) {
        Elf64EHdr_t *eHdr = (Elf64EHdr_t *)location;
        Elf64PHdr_t *pHdr = (Elf64PHdr_t *)(location + eHdr->ePhOff);

        for (unsigned int i = 0; i < eHdr->ePhNum; i ++) {
            if (pHdr[i].pType == PT_LOAD) {
                Addr_t phys = location + pHdr[i].pOffset;
                Addr_t virt = pHdr[i].pVAddr;
                int64_t fSize = pHdr[i].pFileSz;
                int64_t mSize = pHdr[i].pMemSz;

                while (mSize >= 0) {
                    Frame_t f = phys >> 12;

                    if (fSize <= 0) {
                        f = PmmAlloc();
                    }

                    MmuMapPage(virt, f, (pHdr[i].pType&PF_W?PG_WRT:PG_NONE));

                    virt += PAGE_SIZE;
                    phys += PAGE_SIZE;
                    mSize -= PAGE_SIZE;
                    fSize -= PAGE_SIZE;
                }
            }
        }

        rv = eHdr->eEntry;
        MmuUnmapPage(location);
    }

    return rv;
}


