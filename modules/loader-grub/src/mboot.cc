/****************************************************************************************************************//**
*   @file               mboot.cc
*   @brief              Multiboot Parsing Functions
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Jan-03
*   @since              v0.0.01
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*   This file will take care of investigating and retrieving critical information from the multiboot structures.
*   Both Multiboot 1 and Multiboot 2 specifications are supported
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Jan-03 | Initial |  v0.0.01 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "mmu.h"
#include "mboot.h"
#include "serial.h"
#include "boot-interface.h"



/****************************************************************************************************************//**
*   @addtogroup         MB1     Multiboot 1 Structures
*
*   These structures and typedefs are used to overlay on top of the MB1 Multiboot Information Structure
*   @{
*///-----------------------------------------------------------------------------------------------------------------



/****************************************************************************************************************//**
*   @typedef            Mb1Mods_t
*   @brief              Formalization of a loaded module entry.
*
*   @see                Mb1Mods_t
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Mb1Mods_t
*   @brief              Loaded modules entry
*
*   There will be 1 Mb1Mods_t structure for each loaded module.  One of these loaded modules better be the kernel
*   module.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Mb1Mods_t {
    uint32_t modStart;          //!< Module start address
    uint32_t modEnd;            //!< Module end address
    uint32_t modIdent;          //!< Module identifier @note pointer to a string
    uint32_t modReserved;       //!< Reserved word
} __attribute__((packed)) Mb1Mods_t;



/****************************************************************************************************************//**
*   @typedef            Mb1MmapEntry_t
*   @brief              Formalization of a notable block of memory.
*
*   @see                Mb1MmapEntry_t
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Mb1MmapEntry_t
*   @brief              Memory map entry
*
*   At least each usable block of memory will be listed with a Mn1MmapEntry_t structure.  Others may be listed as
*   well, which *may* include ACPI memory which can be reclaimed once all the relevant information has been gleaned
*   from the structures.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Mb1MmapEntry_t {
    uint32_t mmapSize;          //!< Size of this remaining structure block in bytes (after this element)
    uint64_t mmapAddr;          //!< Memory block starting address
    uint64_t mmapLength;        //!< Memory block length
    uint32_t mmapType;          //!< Memory block type (1 == Usable memory)
} __attribute__((packed)) Mb1MmapEntry_t;



/****************************************************************************************************************//**
*   @typedef            Mb1FbRGB_t
*   @brief              Formalization of the bit representation of a pixel's RGB color.
*
*   @see                Mb1FbRGB_t
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Mb1FbRGB_t
*   @brief              Information about the number of bits and where they reside for the RGB palette.
*
*   Defines the location and bitmask for the red, green, and blue bits for a pixel's bits.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Mb1FbRGB_t {
    const uint8_t framebufferRedFieldPos;   //!< The location of the red color bits in the pixel bits
    const uint8_t framebufferRedMaskSize;   //!< The size of the red color bits in the pixel bits
    const uint8_t framebufferGreenFieldPos; //!< The location of the green color bits in the pixel bits
    const uint8_t framebufferGreenMaskSize; //!< The size of the green color bits in the pixel bits
    const uint8_t framebufferBlueFieldPos;  //!< The location of the blue color bits in the pixel bits
    const uint8_t framebufferBlueMaskSize;  //!< The size of the blue color bits in the pixel bits
} __attribute__((packed)) Mb1FbRGB_t;



/****************************************************************************************************************//**
*   @typedef            Mb1FbPalette_t
*   @brief              Formalization of the framebuffer palette.
*
*   @see                Mb1FbPalette_t
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Mb1FbPalette_t
*   @brief              Information about the palette address and size
*
*   Defines the location and size of the palette information
*
*   @note               unused in this implementation
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Mb1FbPalette_t {
    const uint32_t framebufferPaletteAddr;  //!< Pallet address \note unused
    const uint16_t framebufferPaletteNumColors; //!< Pallet color count \note unused
} __attribute__((packed)) Mb1FbPalette_t;



/****************************************************************************************************************//**
*   @typedef            Mb1FbConfig_t
*   @brief              Formalization of the framebuffer palette structures
*
*   @see                Mb1FbConfig_t
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @union              Mb1FbConfig_t
*   @brief              Union of information on how to interpret colors on the display
*
*   Either RPG bits are used or a color palette is used.  This union will provide the necessary data based on which
*   type is configured
*///-----------------------------------------------------------------------------------------------------------------
typedef union Mb1FbConfig_t {
    Mb1FbRGB_t fbRGB;                       //!< The RGB information for each pixel bit
    Mb1FbPalette_t fbPalette;               //!< the pallet structure @note unused
} Mb1FbConfig_t;




/****************************************************************************************************************//**
*   @typedef            MB1_t
*   @brief              Formalization Multiboot 1 Information Structure
*
*   @see                struct MB1_t
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             MB1_t
*   @brief              This is the Multiboot 1 information structure as defined by the spec
*
*   The Multiboot Information Structure conforming to the MB1 specification.  Theoretically, all the information
*   needed by the kernel (x86 anyway) can be gleaned from this structure.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct MB1_t {
    const uint32_t flags;           //!< These flags indicate which data elements have valid data
    const uint32_t availLowerMem;   //!< The basic lower memory limit when flag 0 is set; this value is in KB
    const uint32_t availUpperMem;   //!< The basic upper memory limit when flag 0 is set; this value is in KB
    const uint32_t bootDev;         //!< The boot device when flag 1 is set
    const uint32_t cmdLine;         //!< The command line for this kernel when flag 2 is set \note pointer to a string
    const uint32_t modCount;        //!< The loaded module count when flag 3 is set
    const uint32_t modAddr;         //!< The loaded module list when flag 3 is set \see Mb1Mods_t
    const uint32_t shdrNum;         //!< @brief Section header number of the ELF symbol information when flag 5 is set
                                    //!< @note may still be 0 if not available
    const uint32_t shdrSize;        //!< Section header size of the ELF symbol information when flag 5 is set
    const uint32_t shdrAddr;        //!< Section header address of the ELF symbol information when flag 5 is set
    const uint32_t shdrShndx;       //!< Section header section index of the ELF symbol information when flag 5 is set
    const uint32_t mmapLen;         //!< The memory map structure length when flag 6 is set
    const uint32_t mmapAddr;        //!< The memory map starting address when flag 6 is set
    const uint32_t drivesLen;       //!< The drives structure length when flag 7 is set \note unused
    const uint32_t drivesAddr;      //!< The Drives starting address when flag 7 is set \note unused
    const uint32_t configTable;     //!< The Config table when flag 8 is set \note unused
    const uint32_t bootLoaderName;  //!< The boot loader name when flag 9 is set \note pointer to a string
    const uint32_t apmTable;        //!< The Advanced Power Management table location when bit 10 is set \note unused
    const uint32_t vbeControlInfo;  //!< VBE Control Information Address when bit 11 is set \note unused
    const uint32_t vbeModeInfo;     //!< VBE Mode Information Address when bit 11 is set \note unused
    const uint16_t vbeMode;         //!< VBE Mode when bit 11 is set \note unused
    const uint16_t vbeInterfaceSeg; //!< VBE Interface real mode segment when bit 11 is set \note unused
    const uint16_t vbeInterfaceOff; //!< VBE Interface real mode offset when bit 11 is set \note unused
    const uint16_t vbeInterfaceLen; //!< VBE Interface length when bit 11 is set \note unused
    const uint64_t framebufferAddr; //!< Frame buffer address when bit 12 is set
    const uint32_t framebufferPitch;//!< Frame buffer pitch (width * bpp) when bit 12 is set
    const uint32_t framebufferWidth;//!< Frame buffer width when bit 12 is set
    const uint32_t framebufferHeight;//!< Frame buffer height when bit 12 is set
    const uint8_t framebufferBpp;   //!< Frame buffer bits per pixel when bit 12 is set
    const uint8_t framebufferType;  //!< Frame buffer type when bit 12 is set \note unused
    Mb1FbConfig_t fbConfig;         //!< Frame buffer configuration
} __attribute__((packed)) MB1_t;

/// @}



/****************************************************************************************************************//**
*   @addtogroup         MB2     Multiboot 2 Structures
*
*   These structures and typedefs are used to overlay on top of the MB2 Multiboot Information Structure
*   @{
*///-----------------------------------------------------------------------------------------------------------------



/****************************************************************************************************************//**
*   @typedef            Mb2Fixed_t
*   @brief              A formalization of the fixed Multiboot 2 Information block fixed header.
*
*   @see                Mb2Fixed_t
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Mb2Fixed_t
*   @brief              The fixed header for a Multiboot 2 Information Block.
*
*   This structure identifies the fixed header of the Multiboot Information Block.
*
*   @note               This structure is only used to adjust the pointer to the MBI block.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Mb2Fixed_t {
    uint32_t totalSize;             //!< The total size of the MBI @note unused
    uint32_t reserved;              //!< A reserved dword for structure alignment
} __attribute__((packed)) Mb2Fixed_t;



/****************************************************************************************************************//**
*   @typedef            Mb2BasicTag_t
*   @brief              A formalization of a block's tag information for each block
*
*   @see                Mb2BasicTag_t
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Mb2BasicTag_t
*   @brief              The basic tag header information -- every tag has one
*
*   The basic tag information (header) for each block.
*
*   @note               Each block must be aligned to an 8-byte boundary.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Mb2BasicTag_t {
    uint32_t type;                  //!< The type of the block that follows
    uint32_t size;                  //!< The size of the block
} __attribute__((packed)) Mb2BasicTag_t;



/****************************************************************************************************************//**
*   @typedef            Mb2Module_t
*   @brief              Formalization of a loaded module entry.
*
*   @see                Mb2Module_t
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Mb2Module_t
*   @brief              Loaded modules entry
*
*   There will be 1 Mb2Module_t structure for each loaded module.  One of these loaded modules better be the kernel
*   module.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Mb2Module_t {
    Mb2BasicTag_t tag;              //!< The tag information \note Mb2BasicTag_t::type == 3
    uint32_t modStart;              //!< The start of the module in memory
    uint32_t modEnd;                //!< The end of the module in memory
    char name[0];                   //!< @brief The name of the module \note this is treated as `char *`
                                    //!< @see length also represented by Mb2BasicTag_t::size
} __attribute__((packed)) Mb2Module_t;



/****************************************************************************************************************//**
*   @typedef            Mb2MemMapEntry_t
*   @brief              Formalization of an interesting bit of memory on the system
*
*   @see                Mb2MemMapEntry_t
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Mb2MemMapEntry_t
*   @brief              Memory map entry
*
*   There will be 1 memory map entry for each intersting block of memory on the system.  Not all memory will be
*   reported depending on the system and BIOS.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Mb2MemMapEntry_t {
    uint64_t baseAddr;              //!< The base address of the memory block
    uint64_t length;                //!< The length of the block of memory
    uint32_t type;                  //!< The type of memory reported \note only type == 1 is used
    uint32_t reserved;              //!< Reserved block for alignment
} __attribute__((packed)) Mb2MemMapEntry_t;



/****************************************************************************************************************//**
*   @typedef            Mb2MemMap_t
*   @brief              Formalization of usable memory
*
*   @see                Mb2MemMap_t
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Mb2MemMap_t
*   @brief              Loaded modules entry
*
*   This block reports all the usable memory on the system.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Mb2MemMap_t {
    Mb2BasicTag_t tag;              //!< The tag information \note Mb2BasicTag_t::type == 6
    uint32_t entrySize;             //!< The size (in bytes) of the entire memory map
    uint32_t entryVersion;          //!< The version of the entry? \note unused
    Mb2MemMapEntry_t entries [0];   //!< An unbounded array of the individual entries
} __attribute__((packed)) Mb2MemMap_t;


/// @}



/****************************************************************************************************************//**
*   @var                kernelInterface
*   @brief              The address of the hardware information that is passed between the loader and the kernel
*
*   This structure will hold the information that is passed from  the loader to the kernel.  Physical memory
*   is allocated at runtime and mapped in the MMU at runtime.
*
*   @note               Currently, this structure is limited to 1 page.
*///-----------------------------------------------------------------------------------------------------------------
extern BootInterface_t *kernelInterface;



/****************************************************************************************************************//**
*   @fn                 static Addr_t MBootGetMb1Kernel(void)
*   @brief              When booted by MB1, get the kernel image location
*
*   @returns            The address in physical memory of the kernel module
*
*   @retval             address     The kernel module's address
*   @retval             0           When the kernel is not found
*
*   When booted by an MB1-boot loader, this function will parse the Multiboot Information Structure provided by the
*   boot loader and pull out the relevant information.  Currently, we are only interested in type 3 (modules) and 6
*   (memory map).  If the kernel is found, the address of the module is returned to the caller.
*///-----------------------------------------------------------------------------------------------------------------
static Addr_t MBootGetMb1Kernel(void)
{
    extern uint32_t mbData;
    uint64_t mbd = mbData;
    Addr_t rv = 0;

    MB1_t *mb1 = (MB1_t *)mbd;

    if (!mbData) return 0;

#if DEBUG_ENABLED(MBootGetMb1Kernel)

    SerialPutString("MB1:\n");
    SerialPutString("..mbData is located at ");
    SerialPutHex64(mbData);
    SerialPutChar('\n');

#endif

    if (!MmuIsMapped(mbData)) {
        MmuMapPage(mbData, mbData >> 12, PG_NONE);
    }

#if DEBUG_ENABLED(MBootGetMb1Kernel)

    SerialPutString("MB1 data mapped\n");

#endif


    // -- Handle module information (3)
    if (mb1->flags & (1<<3)) {
        uint64_t mAddr = mb1->modAddr;
        Mb1Mods_t *mods = (Mb1Mods_t *)mAddr;

#if DEBUG_ENABLED(MBootGetMb1Kernel)

        SerialPutString("Module loaded at Address ");
        SerialPutHex64(mAddr);
        SerialPutChar('\n');

#endif

        for (unsigned int i = 0; i < mb1->modCount; i ++) {
            char *name = (char *)((Addr_t)mods[i].modIdent);

            if (!MmuIsMapped((Addr_t)name)) {
                MmuMapPage((Addr_t)name, ((Addr_t)name) >> 12, PG_NONE);
            }

#if DEBUG_ENABLED(MBootGetMb1Kernel)

            SerialPutString(".. ");
            SerialPutString(name);
            SerialPutString(" @ ");
            SerialPutHex64(mods[i].modStart);
            SerialPutChar('\n');

#endif

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

#if DEBUG_ENABLED(MBootGetMb1Kernel)

            SerialPutString(".. Memory Block: ");
            SerialPutHex64(entry->mmapAddr);
            SerialPutString("; size: ");
            SerialPutHex64(entry->mmapLength);
            SerialPutString("; type: ");
            SerialPutHex32(entry->mmapType);
            SerialPutChar('\n');

#endif

            if (entry->mmapType == 1 && cnt < MAX_MEM) {
                kernelInterface->memBlocks[cnt].start = entry->mmapAddr;
                kernelInterface->memBlocks[cnt].end = entry->mmapAddr + entry->mmapLength;

#if DEBUG_ENABLED(MBootGetMb1Kernel)

                SerialPutString(".. (Checking start: ");
                SerialPutHex64(kernelInterface->memBlocks[cnt].start);
                SerialPutString("; end: ");
                SerialPutHex64(kernelInterface->memBlocks[cnt].end);
                SerialPutString(")\n");

#endif

                cnt ++;
            }

            size -= (entry->mmapSize + 4);
            entry = (Mb1MmapEntry_t *)(((Addr_t)entry) + entry->mmapSize + 4);
        }
    }


    return rv;
}



/****************************************************************************************************************//**
*   @fn                 static Addr_t MBootGetMb2Kernel(void)
*   @brief              When booted by MB2, get the kernel image location
*
*   @returns            The address in physical memory of the kernel module
*
*   @retval             address     The kernel module's address
*   @retval             0           When the kernel is not found
*
*   When booted by an MB2-boot loader, this function will parse the Multiboot Information Structure provided by the
*   boot loader and pull out the relevant information.  Currently, we are only interested in type 3 (modules) and 6
*   (memory map).  If the kernel is found, the address of the module is returned to the caller.
*///-----------------------------------------------------------------------------------------------------------------
static Addr_t MBootGetMb2Kernel(void)
{
    extern uint32_t mbData;
    uint32_t locn = (Addr_t)mbData + sizeof(Mb2Fixed_t);
    bool lastTag = false;
    Addr_t rv = 0;

    if (!mbData) return 0;

    SerialPutString("MB2\n");

    if (!MmuIsMapped(mbData)) {
        MmuMapPage(mbData, mbData >> 12, PG_NONE);
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



//*******************************************************************************************************************
//  See `mboot.h`
//-------------------------------------------------------------------------------------------------------------------
Addr_t MBootGetKernel(void)
{
    extern uint32_t mbSig;

#if DEBUG_ENABLED(MBootGetKernel)

    SerialPutString("Checking MB Sig ");
    SerialPutHex32(mbSig);
    SerialPutChar('\n');

#endif

    if (mbSig == MB1SIG) return MBootGetMb1Kernel();
    if (mbSig == MB2SIG) return MBootGetMb2Kernel();
    return 0;
}

