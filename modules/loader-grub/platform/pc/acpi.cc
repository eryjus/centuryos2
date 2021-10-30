/****************************************************************************************************************//**
*   @file               acpi.cc
*   @brief              Functions used to parse the ACPI strutures and gather necessary information
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Oct-24
*   @since              v0.0.12
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*  Much of this file has been copied from the prior 32-bit version of CenturyOS.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Oct-24 | Initial |  v0.0.12 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "mmu.h"
#include "serial.h"
#include "boot-interface.h"
#include "discovery.h"



/****************************************************************************************************************//**
*   @def                MAKE_SIG
*   @brief              Macro to convert a char[4] into a uint_32
*///-----------------------------------------------------------------------------------------------------------------
#define MAKE_SIG(s)         ((uint32_t)(s[3]<<24)|(uint32_t)(s[2]<<16)|(uint32_t)(s[1]<<8)|s[0])



/****************************************************************************************************************//**
*   @def                RSDP_SIG
*   @brief              This is the signature of the RSDP, expressed as a uint64_t
*///-----------------------------------------------------------------------------------------------------------------
#define RSDP_SIG            ((uint64_t)' '<<56|(uint64_t)'R'<<48|(uint64_t)'T'<<40|(uint64_t)'P'<<32\
                    |(uint64_t)' '<<24|(uint64_t)'D'<<16|(uint64_t)'S'<<8|(uint64_t)'R')




/****************************************************************************************************************//**
*   @enum               MadtIcType
*   @brief              These are the types of Interrupt Controller Structure Types we can have
*///-----------------------------------------------------------------------------------------------------------------
typedef enum {
    MADT_PROCESSOR_LOCAL_APIC = 0,          //!< Processor Local APIC
    MADT_IO_APIC = 1,                       //!< IO APIC
    MADT_INTERRUPT_SOURCE_OVERRIDE = 2,     //!< Interrupt Source Override
    MADT_NMI_SOURCE = 3,                    //!< NMI Source
    MADT_LOCAL_APIC_NMI = 4,                //!< Local APIC NMI
    MADT_LOCAL_APIC_ADDRESS_OVERRIDE = 5,   //!< Local APIC Address Override
    MADT_IO_SAPIC = 6,                      //!< Streamlined IO APIC ?
    MADT_LOCAL_SAPIC = 7,                   //!< Local Streamlined APIC
    MADT_PLATFORM_INTERRUPT_SOURCES = 8,    //!< Platform Interrupt Sources
    MADT_PROCESSOR_LOCAL_X2APIC = 9,        //!< Local Processor X2APIC
    MADT_LOCAL_X2APIC_NMI = 0xa,            //!< X2APIC NMI
    MADT_GIC = 0xb,                         //!< Generic Interrupt Controller
    MADT_GICD = 0xc,                        //!< GIC Distributor
} MadtIcType;


/****************************************************************************************************************//**
*   @typedef            RsdpSig_t
*   @brief              A formalization of the structure of the RSDP signature
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             RsdpSig_t
*   @brief              The signature for locating the RSDP table
*///-----------------------------------------------------------------------------------------------------------------
typedef union RsdpSig_t {
    char signature[8];              //!< The character signature "RSD PTR " \note Notice the trailing space
    uint64_t lSignature;            //!< The same signature characters represented as a 64-bit unsigned integer
} __attribute__((packed)) RsdpSig_t;



/****************************************************************************************************************//**
*   @typedef            Rsdp_t
*   @brief              A formalization of the structure of the RSDP table
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Rsdp_t
*   @brief              The structure known as the RSDP (Root System Description Pointer)
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Rsdp_t {
    RsdpSig_t sig;                  //!< The RSDP signature \see RsdpSig_t
    uint8_t checksum;               //!< The checksum byte which makes the structure sum to 0 (last byte of the sum)
    char oemid[6];                  //!< The OEM id
    uint8_t revision;               //!< Which ACPI revision
    uint32_t rsdtAddress;           //!< The address of the actual structures (32-bit)
    uint32_t length;                //!< The length of the tables
    uint64_t xsdtAddress;           //!< The address of the actual structures (64-bit)
    uint8_t extendedChecksum;       //!< The checksum byte for the extra fields for 64-bit
    uint8_t reserved[3];            //!< Reserved bytes
} __attribute__((packed, aligned(16))) Rsdp_t;



/****************************************************************************************************************//**
*   @typedef            AcpiSig_t
*   @brief              A formalization of the structure of the ACPI table signature
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             AcpiSig_t
*   @brief              The signature used to determine which ACPI table is being examined
*///-----------------------------------------------------------------------------------------------------------------
typedef union AcpiSig_t {
    char signature[4];              //!< The signature bytes
    uint32_t lSignature;            //!< The same signature characters represented as a 32-bit unsigned integer
} __attribute__((packed)) AcpiSig_t;



/****************************************************************************************************************//**
*   @typedef            AcpiStdHdr_t
*   @brief              A formalization of the structure of the standard ACPI table header
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             AcpiStdHdr_t
*   @brief              The standard ACPI table header found on all tables
*///-----------------------------------------------------------------------------------------------------------------
typedef struct AcpiStdHdr_t {
    AcpiSig_t sig;                  //!< The table signature
    uint32_t length;                //!< The table length
    uint8_t revision;               //!< The version number to which the table conforms
    uint8_t checksum;               //!< The checksum byte which makes the structure sum to 0 (last byte of the sum)
    char oemid[6];                  //!< The OEM id
    uint64_t oemTableId;            //!< The OEM Table ID
    uint32_t oemRevision;           //!< The OEM Revision Level
    uint32_t creatorId;             //!< The creator ID
    uint32_t creatorRevision;       //!< The creator revision Level
} __attribute__((packed)) AcpiStdHdr_t;



/****************************************************************************************************************//**
*   @typedef            Xsdt_t
*   @brief              A formalization of the XSDT table structure
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Xsdt_t
*   @brief              The 64-bit XSDT structure used for determining system hardware
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Xsdt_t {
    AcpiStdHdr_t hdr;               //!< The standard ACPI table header
    uint64_t entry[0];              //!< there may be 0 or several entries; length must be checked
} __attribute__((packed)) Xsdt_t;


/****************************************************************************************************************//**
*   @typedef            Rsdt_t
*   @brief              A formalization of the RSDT table structure
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Rsdt_t
*   @brief              This is the Root System Description Table (RSDT)
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Rsdt_t {
    AcpiStdHdr_t hdr;               //!< The standard ACPI table header
    uint32_t entry[0];              //!< there may be 0 or several of these; length must be checked
} __attribute__((packed)) Rsdt_t;



/****************************************************************************************************************//**
*   @typedef            MADT_t
*   @brief              A formalization of the MADT table structure
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             MADT_t
*   @brief              The Multiple APIC Description Table (MADT)
*///-----------------------------------------------------------------------------------------------------------------
typedef struct MADT_t {
    AcpiStdHdr_t hdr;               //!< The standard ACPI table header
    uint32_t localIntCtrlAddr;      //!< Local interrupt controller address
    uint32_t flags;                 //!< Flags
    uint8_t intCtrlStructs[0];      //!< Interrupt Controller Structures
} __attribute__((packed)) MADT_t;



/****************************************************************************************************************//**
*   @typedef            MadtLocalApic_t
*   @brief              A formalization of the Local Processor APIC structure
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             MadtLocalApic_t
*   @brief              Local Processor APIC structure
*///-----------------------------------------------------------------------------------------------------------------
typedef struct MadtLocalApic_t {
    uint8_t type;                   //!< \ref MADT_PROCESSOR_LOCAL_APIC
    uint8_t len;                    //!< Length in bytes
    uint8_t procId;                 //!< Processor ID
    uint8_t apicId;                 //!< APIC ID
    uint32_t flags;                 //!< flags \note 0b00000001 means the processor is enabled
} __attribute__((packed)) MadtLocalApic_t;



/****************************************************************************************************************//**
*   @typedef            MadtIoApic_t
*   @brief              A formalization of the IO APIC structure
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             MadtIoApic_t
*   @brief              I/O APIC Structure
*///-----------------------------------------------------------------------------------------------------------------
typedef struct MadtIoApic_t {
    uint8_t type;                   //!< \ref MADT_IO_APIC
    uint8_t len;                    //!< Length in bytes
    uint8_t apicId;                 //!< APIC ID
    uint8_t reserved;               //!< Reserved byte
    uint32_t ioApicAddr;            //!< IO APIC Address in memory
    uint32_t gsiBase;               //!< Global System Interrupt Number where this APIC starts
} __attribute__((packed)) MadtIoApic_t;



/****************************************************************************************************************//**
*   @typedef            MadtIntSrcOverride_t
*   @brief              A formalization of the Interrupt Source Override Structure
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             MadtIntSrcOverride_t
*   @brief              Interrupt Source Override Structure
*///-----------------------------------------------------------------------------------------------------------------
typedef struct MadtIntSrcOverride_t {
    uint8_t type;                   //!< \ref MADT_INTERRUPT_SOURCE_OVERRIDE
    uint8_t len;                    //!< Length in bytes
    uint8_t bus;                    //!< fixed: 0 = ISA
    uint8_t source;                 //!< IRQ Source
    uint32_t gsInt;                 //!< Global System Interrupt number
    uint32_t flags;                 //!< Flags
} __attribute__((packed)) MadtIntSrcOverride_t;



/****************************************************************************************************************//**
*   @typedef            MadtMNISource_t
*   @brief              A formalization of the NMI Interrupt Source Structure
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             MadtMNISource_t
*   @brief              NMI Interrupt Source Structure
*///-----------------------------------------------------------------------------------------------------------------
typedef struct MadtMNISource_t {
    uint8_t type;                   //!< \ref MADT_NMI_SOURCE
    uint8_t len;                    //!< Length in bytes
    uint16_t flags;                 //!< Flags
    uint32_t gsInt;                 //!< Global System Interrupt
} __attribute__((packed)) MadtMNISource_t;



/****************************************************************************************************************//**
*   @typedef            MadtLocalApicNMI_t
*   @brief              A formalization of the Local APIC NMI Structure
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             MadtLocalApicNMI_t
*   @brief              Local APIC NMI Structure
*///-----------------------------------------------------------------------------------------------------------------
typedef struct MadtLocalApicNMI_t {
    uint8_t type;                   //!< \ref MADT_LOCAL_APIC_NMI
    uint8_t len;                    //!< Length in bytes
    uint8_t procId;                 //!< 0xff is all processors
    uint16_t flags;                 //!< Flags
    uint8_t localLINT;              //!< Local APIC interrupt input LINTn to which NMI is connected
} __attribute__((packed)) MadtLocalApicNMI_t;



/****************************************************************************************************************//**
*   @var                rsdp
*   @brief              The location of the RSDP when found
*///-----------------------------------------------------------------------------------------------------------------
Rsdp_t *rsdp = NULL;



/****************************************************************************************************************//**
*   @fn                 static bool IsRsdp(Rsdp_t *rsdp)
*   @brief              Validate the RSDP checksum is accurate
*
*   Loop through all the RSDP and add it's values up.  The lowest 8 bits must be 0 for the checksum to be valid.
*
*   @param              rsdp            Pointer to the candidate RSDP
*   @returns            whether the RSDP is valid
*
*   @retval             false           The RSDP is not valid
*   @retval             true            The RSDP is valid
*///-----------------------------------------------------------------------------------------------------------------
static bool IsRsdp(Rsdp_t *rsdp)
{
    if (!rsdp) return false;

    uint32_t cs = 0;
    uint8_t *data = (uint8_t *)rsdp;

    for (uint32_t i = 0; i < 20; i ++) {
        cs += data[i];
    }

    return (cs & 0xff) == 0;
}



/****************************************************************************************************************//**
*   @fn                 static Rsdp_t *AcpiFindRsdp(void)
*   @brief              Locate the Root System Description Table
*
*   Search all the known locations for the RSDP and if found, set its location
*
*   @returns            the location of the RSDP, NULL if not found
*
*   @retval             NULL            when the RSDP table is not found
*   @retval             non-NULL        the address of the RSDP
*///-----------------------------------------------------------------------------------------------------------------
static Rsdp_t *AcpiFindRsdp(void)
{
    Addr_t wrk = EBDA & ~0x000f;
    Addr_t end = wrk + 1024;
    Addr_t pg = wrk & ~(PAGE_SIZE - 1);
    Rsdp_t *rsdp;
    Rsdp_t *rv = NULL;


#if DEBUG_ENABLED(AcpiFindRsdp)

    SerialPutString("Searching for the RSDP\n");

#endif

    ldr_MmuMapPage(pg, pg >> 12, PG_KRN);
    if (wrk != 0) {
        while (wrk < end) {
            rsdp = (Rsdp_t *)wrk;

            if (rsdp->sig.lSignature == RSDP_SIG && IsRsdp(rsdp)) {

#if DEBUG_ENABLED(AcpiFindRsdp)

                SerialPutString("RSDP found at address ");
                SerialPutHex64(wrk);
                SerialPutChar('\n');

#endif

                rsdp = (Rsdp_t *)wrk;
                ldr_MmuUnmapPage(pg);
                return rsdp;
            }

            wrk += 16;
        }
    }
    ldr_MmuUnmapPage(pg);

#if DEBUG_ENABLED(AcpiFindRsdp)

    SerialPutString("Not found in the EBDA; checking BIOS\n");

#endif

    wrk = BIOS;
    end = BIOS_END;

    for (pg = wrk; pg < end; pg += PAGE_SIZE) {
        ldr_MmuMapPage(pg, pg >> 12, PG_KRN);
    }
    pg = wrk;       // reset pg

    while (wrk < end) {

#if DEBUG_ENABLED(AcpiFindRsdp)

            SerialPutString("... checking at ");
            SerialPutHex64(wrk);
            SerialPutChar('\n');

#endif


        rsdp = (Rsdp_t *)wrk;

        if (rsdp->sig.lSignature == RSDP_SIG && IsRsdp(rsdp)) {

#if DEBUG_ENABLED(AcpiFindRsdp)

            SerialPutString("RSDP found at address ");
            SerialPutHex64(wrk);
            SerialPutString("\n.. Version ");
            SerialPutHex32((uint32_t)rsdp->revision);
            SerialPutString("\n.. Rsdt Address ");
            SerialPutHex32(rsdp->rsdtAddress);
            SerialPutString("\n.. Xsdt Address ");
            SerialPutHex64(rsdp->xsdtAddress);
            SerialPutChar('\n');

#endif

            rsdp = (Rsdp_t *)wrk;
            rv = rsdp;
            goto exit;
        }

        wrk += 16;
    }

exit:
    for (pg = wrk; pg < end; pg += PAGE_SIZE) {
        ldr_MmuUnmapPage(pg);
    }

#if DEBUG_ENABLED(AcpiFindRsdp)

    SerialPutString("Returning RSDP at ");
    SerialPutHex64((Addr_t)rv);
    SerialPutChar('\n');

#endif

    return rv;
}



/****************************************************************************************************************//**
*   @fn                 static bool AcpiCheckTable(Addr_t loc, uint32_t sig)
*   @brief              Check the table to see if it is what we expect
*
*   Does the ACPI table at the location provided contain the signature we are looking for?
*
*   @returns            whether the table contains the signature we desire
*
*   @retval             true            This is the desired table
*   @retval             false           This is not the desired table
*
*   @note Memory must be mapped before calling
*///-----------------------------------------------------------------------------------------------------------------
static bool AcpiCheckTable(Addr_t loc, uint32_t sig)
{
    uint8_t *table = (uint8_t *)loc;
    uint32_t size;
    Addr_t checksum = 0;
    bool needUnmap = false;
    Addr_t page = loc & ~(PAGE_SIZE - 1);

    if (!MmuIsMapped(page)) {
        ldr_MmuMapPage(page, page >> 12, PG_KRN);
        needUnmap = true;
    }

#if DEBUG_ENABLED(AcpiCheckTable)

    SerialPutString(".. Checking the ACPI table....\n....");
    SerialPutHex32(*(uint32_t *)loc);
    SerialPutString(" vs ");
    SerialPutHex32(sig);
    SerialPutChar('\n');

#endif

    if (*((uint32_t *)loc) != sig) {

#if DEBUG_ENABLED(AcpiCheckTable)

        SerialPutString(".. (signature check fails)\n");

#endif
        if (needUnmap) ldr_MmuUnmapPage(page);

        return false;
    }

    size = *((uint32_t *)(loc + 4));

#if DEBUG_ENABLED(AcpiCheckTable)

    SerialPutString(".. Checking ");
    SerialPutHex32(size);
    SerialPutString(" bytes of the table\n");

#endif

    for (uint32_t i = 0; i < size; i ++) {
        Addr_t wrk = (Addr_t)(&table[i]);
        if (!MmuIsMapped(wrk)) {
            ldr_MmuMapPage(wrk, wrk >> 12, PG_KRN);
        }
        checksum += table[i];
    }

    if (needUnmap) ldr_MmuUnmapPage(page);

    return (checksum & 0xff) == 0;
}



/****************************************************************************************************************//**
*   @fn                 static void AcpiReadMadt(Addr_t loc, BootInterface_t *hw)
*   @brief              Read the ACPI MADT Table, and figure out what it means to CenturyOS
*
*   Read the MADT table and determine what the APICs the system has that we might be interested in.
*
*   @param              loc         The location of the MADT table
*   @param              hw          Pointer to the hardware interface table containing items of interest
*
*   @note Memory must be mapped before calling
*///-----------------------------------------------------------------------------------------------------------------
static void AcpiReadMadt(Addr_t loc, BootInterface_t *hw)
{
    MADT_t *madt = (MADT_t *)loc;

#if DEBUG_ENABLED(AcpiReadMadt)

    SerialPutString(".... MADT table length is ");
    SerialPutHex32(madt->hdr.length);
    SerialPutString("\n.... MADT flags are ");
    SerialPutHex32(madt->flags);
    SerialPutString("\n.... MADT Local IC Address is ");
    SerialPutHex32(madt->localIntCtrlAddr);
    SerialPutChar('\n');

#endif

    uint8_t *wrk = (uint8_t *)(loc + MEMBER_OFFSET(MADT_t,intCtrlStructs));
    uint8_t *first = wrk;

    while (wrk - first < (long)(madt->hdr.length - MEMBER_OFFSET(MADT_t,intCtrlStructs))) {
        uint8_t len = wrk[1];

        switch(wrk[0]) {
        case MADT_PROCESSOR_LOCAL_APIC:
            {
                hw->cpuCount ++;
                hw->localApic ++;

#if DEBUG_ENABLED(AcpiReadMadt)
                MadtLocalApic_t *local = (MadtLocalApic_t *)wrk;

                SerialPutString(".... MADT_PROCESSOR_LOCAL_APIC\n");
                SerialPutString("...... Proc ID ");
                SerialPutHex32(local->procId);
                SerialPutString("; APIC ID ");
                SerialPutHex32(local->apicId);
                SerialPutString(" ; ");
                SerialPutString(local->flags&1?"enabled":"disabled");
                SerialPutString(" (");
                SerialPutHex32(hw->cpuCount);
                SerialPutString(" found so far)\n");

#endif
            }

            break;

        case MADT_IO_APIC:
            {
#if DEBUG_ENABLED(AcpiReadMadt)
                MadtIoApic_t *local = (MadtIoApic_t *)wrk;

                SerialPutString(".... MADT_IO_APIC\n");
                SerialPutString("...... APIC Addr: ");
                SerialPutHex64(local->ioApicAddr);
                SerialPutString(", Global Sys Int Base: ");
                SerialPutHex32(local->gsiBase);
                SerialPutChar('\n');

#endif
            }

            break;

        case MADT_INTERRUPT_SOURCE_OVERRIDE:
            {
#if DEBUG_ENABLED(AcpiReadMadt)
                MadtIntSrcOverride_t *local = (MadtIntSrcOverride_t *)wrk;

                SerialPutString(".... MADT_INTERRUPT_SOURCE_OVERRIDE\n");
                SerialPutString("...... source: ");
                SerialPutHex32(local->source);
                SerialPutString(", Global Sys Int: ");
                SerialPutHex32(local->gsInt);
                SerialPutString("\n...... Polarity: ");
                SerialPutHex32(local->flags & 0x03);
                SerialPutString("; Trigger Mode: ");
                SerialPutHex32((local->flags >> 2) & 0x03);
                SerialPutChar('\n');

#endif
            }

            break;

        case MADT_NMI_SOURCE:
            {
#if DEBUG_ENABLED(AcpiReadMadt)
                MadtMNISource_t *local = (MadtMNISource_t *)wrk;

                SerialPutString(".... MADT_NMI_SOURCE\n");
                SerialPutString("Global Sys Int: ");
                SerialPutHex32(local->gsInt);
                SerialPutString("\n...... Polarity: ");
                SerialPutHex32(local->flags & 0x03);
                SerialPutString("; Trigger Mode: ");
                SerialPutHex32((local->flags >> 2) & 0x03);
                SerialPutChar('\n');

#endif

            }

            break;

        case MADT_LOCAL_APIC_NMI:
            {
#if DEBUG_ENABLED(AcpiReadMadt)
                MadtLocalApicNMI_t *local = (MadtLocalApicNMI_t *)wrk;

                SerialPutString(".... MADT_LOCAL_APIC_NMI\n");
                SerialPutString("...... APIC Proc ID: ");
                SerialPutHex32(local->procId);
                SerialPutString("; local INT#: ");
                SerialPutHex32(local->localLINT);
                SerialPutString("\n...... Polarity: ");
                SerialPutHex32(local->flags & 0x03);
                SerialPutString("; Trigger Mode: ");
                SerialPutHex32((local->flags >> 2) & 0x03);
                SerialPutChar('\n');

#endif
            }

            break;

        case MADT_LOCAL_APIC_ADDRESS_OVERRIDE:
            SerialPutString("!!!! MADT IC Table Type MADT_LOCAL_APIC_ADDRESS_OVERRIDE is not supported\n");
            break;

        case MADT_IO_SAPIC:
            SerialPutString("!!!! MADT IC Table Type MADT_IO_SAPIC is not supported\n");
            break;

        case MADT_LOCAL_SAPIC:
            SerialPutString("!!!! MADT IC Table Type MADT_LOCAL_SAPIC is not supported\n");
            break;

        case MADT_PLATFORM_INTERRUPT_SOURCES:
            SerialPutString("!!!! MADT IC Table Type MADT_PLATFORM_INTERRUPT_SOURCES is not supported\n");
            break;

        case MADT_PROCESSOR_LOCAL_X2APIC:
            SerialPutString("!!!! MADT IC Table Type MADT_PROCESSOR_LOCAL_X2APIC is not supported\n");
            break;

        case MADT_LOCAL_X2APIC_NMI:
            SerialPutString("!!!! MADT IC Table Type MADT_LOCAL_X2APIC_NMI is not supported\n");
            break;

        case MADT_GIC:
            SerialPutString("!!!! MADT IC Table Type GIC is not supported\n");
            break;

        case MADT_GICD:
            SerialPutString("!!!! MADT IC Table Type GICD is not supported\n");
            break;

        default:
            SerialPutString("!!!! Unknown MADT IC Table Type: ");
            SerialPutHex32(wrk[0]);
            SerialPutChar('\n');
            break;
        }


        wrk += len;
    }
}



/****************************************************************************************************************//**
*   @fn                 static uint32_t AcpiGetTableSig(Addr_t loc, BootInterface_t *hw)
*   @brief              Get the table signature (and check its valid); return 0 if invalid
*
*   Determine what table the current table is and if it of interest, parse its data.
*
*   @param              loc         The location of the MADT table
*   @param              hw          Pointer to the hardware interface table containing items of interest
*
*   @note This function will map and unmap memory as needed to satisfy access to the memory
*///-----------------------------------------------------------------------------------------------------------------
static uint32_t AcpiGetTableSig(Addr_t loc, BootInterface_t *hw)
{
    if (!loc) return 0;

    bool needUnmap = false;
    Addr_t page = loc & ~(PAGE_SIZE - 1);

#if DEBUG_ENABLED(AcpiGetTableSig)

    SerialPutString("Checking ACPI table at ");
    SerialPutHex64(loc);
    SerialPutChar('\n');

#endif

    if (!MmuIsMapped(page)) {
        ldr_MmuMapPage(page, page >> 12, PG_KRN);
        needUnmap = true;
    }

    uint32_t rv = *((uint32_t *)loc);

    if (!AcpiCheckTable(loc, rv)) {
        rv = 0;
        goto exit;
    }

    switch(rv) {
    case MAKE_SIG("APIC"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. APIC: Multiple APIC Description Table\n");

#endif

        AcpiReadMadt(loc, hw);
        break;

    case MAKE_SIG("BERT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. BERT: Boot Error Record Table\n");

#endif

        break;

    case MAKE_SIG("BGRT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. BGRT: Boot Graphics Resource Table\n");

#endif

        break;

    case MAKE_SIG("BOOT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. BOOT: Simple Boot Flag Table\n");

#endif

        break;

    case MAKE_SIG("CPEP"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. CPEP: Corrected Platform Error Polling Table\n");

#endif

        break;

    case MAKE_SIG("CSRT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. CSRT: Core System Resource Table\n");

#endif

        break;

    case MAKE_SIG("DBG2"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. DBG2: Debug Port Table 2\n");

#endif

        break;

    case MAKE_SIG("DBGP"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. DBGP: Debug Port Table\n");

#endif

        break;

    case MAKE_SIG("DMAR"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. DMAR: DMA Remapping Table\n");

#endif

        break;

    case MAKE_SIG("DRTM"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. DRTM: Dynamic Root of Trust for Measurement Table\n");

#endif

        break;

    case MAKE_SIG("DSDT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. DSDT: Differentiated System Description Table\n");

#endif

        break;

    case MAKE_SIG("ECDT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. ECDT: Embedded Controller Boot Resources Table\n");

#endif

        break;

    case MAKE_SIG("EINJ"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. EINJ: Error Injection Table\n");

#endif

        break;

    case MAKE_SIG("ERST"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. ERST: Error Record Serialization Table\n");

#endif

        break;

    case MAKE_SIG("ETDT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. ETDT: Event Timer Description Table (Obsolete)\n");

#endif

        break;

    case MAKE_SIG("FACP"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. FACP: Fixed ACPI Description Table (FADT)\n");

#endif

        break;

    case MAKE_SIG("FACS"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. FACS: Firmware ACPI Control Structure\n");

#endif

        break;

    case MAKE_SIG("FPDT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. FPDT: Firmware Performance Data Table\n");

#endif

        break;

    case MAKE_SIG("GTDT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. GTDT: Generic Timer Description Table\n");

#endif

        break;

    case MAKE_SIG("HEST"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. HEST: Hardware Error Source Table\n");

#endif

        break;

    case MAKE_SIG("HPET"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. HPET: High Performance Event Timer\n");

#endif

        break;

    case MAKE_SIG("IBFT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. IBFT: iSCSI Boot Firmware Table\n");

#endif

        break;

    case MAKE_SIG("IVRS"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. IVRS: I/O Virtualization Reporting Structure\n");

#endif

        break;

    case MAKE_SIG("MCFG"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. MCFG: PCI Express memory mapped configuration space base address Description Table\n");

#endif

        break;

    case MAKE_SIG("MCHI"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. MCHI: Management Controller Host Interface Table\n");

#endif

        break;

    case MAKE_SIG("MPST"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. MPST: Memory Power State Table\n");

#endif

        break;

    case MAKE_SIG("MSCT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. MSCT: Maximum System Characteristics Table");

#endif

        break;

    case MAKE_SIG("MSDM"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. MSDM: Microsoft Data Management Table\n");

#endif

        break;

    case MAKE_SIG("OEM0"):
    case MAKE_SIG("OEM1"):
    case MAKE_SIG("OEM2"):
    case MAKE_SIG("OEM3"):
    case MAKE_SIG("OEM4"):
    case MAKE_SIG("OEM5"):
    case MAKE_SIG("OEM6"):
    case MAKE_SIG("OEM7"):
    case MAKE_SIG("OEM8"):
    case MAKE_SIG("OEM9"):
    case MAKE_SIG("OEMA"):
    case MAKE_SIG("OEMB"):
    case MAKE_SIG("OEMC"):
    case MAKE_SIG("OEMD"):
    case MAKE_SIG("OEME"):
    case MAKE_SIG("OEMF"):
    case MAKE_SIG("OEMG"):
    case MAKE_SIG("OEMH"):
    case MAKE_SIG("OEMI"):
    case MAKE_SIG("OEMJ"):
    case MAKE_SIG("OEMK"):
    case MAKE_SIG("OEML"):
    case MAKE_SIG("OEMM"):
    case MAKE_SIG("OEMN"):
    case MAKE_SIG("OEMO"):
    case MAKE_SIG("OEMP"):
    case MAKE_SIG("OEMQ"):
    case MAKE_SIG("OEMR"):
    case MAKE_SIG("OEMS"):
    case MAKE_SIG("OEMT"):
    case MAKE_SIG("OEMU"):
    case MAKE_SIG("OEMV"):
    case MAKE_SIG("OEMW"):
    case MAKE_SIG("OEMX"):
    case MAKE_SIG("OEMY"):
    case MAKE_SIG("OEMZ"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. OEMx: OEM Specific Information Table\n");

#endif

        break;

    case MAKE_SIG("PMTT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. PMTT: Platform Memory Topology Table\n");

#endif

        break;

    case MAKE_SIG("PSDT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. PSDT: Persistent System Description Table\n");

#endif

        break;

    case MAKE_SIG("RASF"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. RASF: ACPI RAS Feature Table\n");

#endif

        break;

    case MAKE_SIG("RSDT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. RSDT: Root System Description Table\n");

#endif

        break;

    case MAKE_SIG("SBST"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. SBST: Smart Battery Specification Table\n");

#endif

        break;

    case MAKE_SIG("SLIC"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. SLIC: Microsoft Software Licensing Table Specification\n");

#endif

        break;

    case MAKE_SIG("SLIT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. SLIT: System Locality Distance Information Table\n");

#endif

        break;

    case MAKE_SIG("SPCR"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. SPCR: Serial Port Console Redirection Table\n");

#endif

        break;

    case MAKE_SIG("SPMI"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. SPMI: Server Platform Management Interface Table\n");

#endif

        break;

    case MAKE_SIG("SRAT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. SRAT: System Resource Affinity Table\n");

#endif

        break;

    case MAKE_SIG("SSDT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. SSDT: Secondary System Description Table\n");

#endif

        break;

    case MAKE_SIG("TCPA"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. TCPA: Trusted Computing Platform Alliance Capabilities Table\n");

#endif

        break;

    case MAKE_SIG("TPM2"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. TPM2: Trusted Platform Module 2 Table\n");

#endif

        break;

    case MAKE_SIG("UEFI"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. UEFI: UEFI ACPI Data Table\n");

#endif

        break;

    case MAKE_SIG("WAET"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. WAET: Windows ACPI Emulated Deviced Table\n");

#endif

        break;

    case MAKE_SIG("WDAT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. WDAT: Watch Dog Action Table\n");

#endif

        break;

    case MAKE_SIG("XSDT"):
#if DEBUG_ENABLED(AcpiGetTableSig)

        SerialPutString(".. XSDT: Extended System Description Table\n");

#endif

        break;

    default:
        {
            char l1 [2] = {0};
            char l2 [2] = {0};
            char l3 [2] = {0};
            char l4 [2] = {0};

            l4[0] = (rv >> 24) & 0xff;
            l3[0] = (rv >> 16) & 0xff;
            l2[0] = (rv >> 8) & 0xff;
            l1[0] = rv & 0xff;

#if DEBUG_ENABLED(AcpiGetTableSig)

            SerialPutString(".. The table for this signature is invalid; ignoring table ");
            SerialPutString(l1);
            SerialPutString(l2);
            SerialPutString(l3);
            SerialPutString(l4);
            SerialPutChar('\n');

#endif

            rv = 0;

            goto exit;
        }
    }

exit:
    if (needUnmap) MmuUnmapPage(page);

    return rv;
}



/****************************************************************************************************************//**
*   @fn                 static bool AcpiReadXsdt(Addr_t loc, BootInterface_t *hw)
*   @brief              Read the XSDT table
*
*   Read the RSDT/XSDT table and determine if the table is really the XSDT table.  Rerturn if the XSDT table was
*   found or not.
*
*   @param              loc         The location of the table
*   @param              hw          Pointer to the hardware interface table containing items of interest
*
*   @returns            whether the location contained the XSDT table
*
*   @retval             true        The table at loc is the XSDT
*   @retval             false       The table at loc is not the XSDT
*
*   @note This function will map and unmap memory as needed to satisfy access to the memory
*///-----------------------------------------------------------------------------------------------------------------
static bool AcpiReadXsdt(Addr_t loc, BootInterface_t *hw)
{
    if (!loc) return false;
    bool rv = true;
    Addr_t page = loc & ~(PAGE_SIZE - 1);
    Xsdt_t *xsdt = (Xsdt_t *)loc;
    uint32_t entries = 0;

#if DEBUG_ENABLED(AcpiReadXsdt)

    SerialPutString("Reading the XSDT\n");

#endif

    ldr_MmuMapPage(page, page >> 12, PG_KRN);

    if (!AcpiCheckTable(loc, MAKE_SIG("XSDT"))) {

#if DEBUG_ENABLED(AcpiReadXsdt)

        SerialPutString("The XSDT does not match the required checks\n");

#endif

        rv =  false;
        goto exit;
    }

    entries = (xsdt->hdr.length - sizeof(AcpiStdHdr_t)) / sizeof(uint64_t);

#if DEBUG_ENABLED(AcpiReadXsdt)

    SerialPutString("... checking ");
    SerialPutHex32(entries);
    SerialPutString(" entries\n");

#endif

    for (uint32_t i = 0; i < entries; i ++) {

#if DEBUG_ENABLED(AcpiReadXsdt)

        SerialPutString("The address for entry ");
        SerialPutHex32(i);
        SerialPutString(" is ");
        SerialPutHex64(xsdt->entry[i]);
        SerialPutChar('\n');

#endif

        if (xsdt->entry[i]) AcpiGetTableSig(xsdt->entry[i], hw);
    }

exit:
    ldr_MmuUnmapPage(page);
    return rv;
}



/****************************************************************************************************************//**
*   @fn                 static bool AcpiReadRsdt(Addr_t loc, BootInterface_t *hw)
*   @brief              Read the RSDT table
*
*   Read the RSDT/XSDT table and determine if the table is really the RSDT table.  Rerturn if the RSDT table was
*   found or not.
*
*   @param              loc         The location of the table
*   @param              hw          Pointer to the hardware interface table containing items of interest
*
*   @returns            whether the location contained the RSDT table
*
*   @retval             true        The table at loc is the RSDT
*   @retval             false       The table at loc is not the RSDT
*
*   @note This function will map and unmap memory as needed to satisfy access to the memory
*///-----------------------------------------------------------------------------------------------------------------
static bool AcpiReadRsdt(Addr_t loc, BootInterface_t *hw)
{
    if (!loc) return false;
    bool rv = true;
    Addr_t page = loc & ~(PAGE_SIZE - 1);
    Rsdt_t *rsdt = (Rsdt_t *)loc;
    uint32_t entries = 0;

#if DEBUG_ENABLED(AcpiReadRsdt)

    SerialPutString("Reading the RSDT\n");

#endif

    ldr_MmuMapPage(page, page >> 12, PG_KRN);

    if (!AcpiCheckTable(loc, MAKE_SIG("RSDT"))) {

#if DEBUG_ENABLED(AcpiReadRsdt)

        SerialPutString("The RSDT does not match the required checks\n");

#endif

        rv =  false;
        goto exit;
    }

    entries = (rsdt->hdr.length - sizeof(AcpiStdHdr_t)) / sizeof(uint32_t);

#if DEBUG_ENABLED(AcpiReadRsdt)

    SerialPutString("... checking ");
    SerialPutHex32(entries);
    SerialPutString(" entries\n");

#endif

    for (uint32_t i = 0; i < entries; i ++) {

#if DEBUG_ENABLED(AcpiReadRsdt)

        SerialPutString("The address for entry ");
        SerialPutHex32(i);
        SerialPutString(" is ");
        SerialPutHex32(rsdt->entry[i]);
        SerialPutChar('\n');

#endif

        if (rsdt->entry[i]) AcpiGetTableSig(rsdt->entry[i], hw);
    }

exit:
    ldr_MmuUnmapPage(page);
    return rv;
}



/********************************************************************************************************************
* -- Documented in discovery.h
*///-----------------------------------------------------------------------------------------------------------------
void PlatformDiscovery(BootInterface_t *hw)
{
    rsdp = AcpiFindRsdp();
    Addr_t page = ((Addr_t)rsdp) & ~(PAGE_SIZE - 1);

    ldr_MmuMapPage(page, page >> 12, PG_KRN);

    if (AcpiCheckTable(rsdp->xsdtAddress, MAKE_SIG("XSDT"))) {
        AcpiReadXsdt(rsdp->xsdtAddress, hw);
    } else if (AcpiCheckTable(rsdp->rsdtAddress, MAKE_SIG("RSDT"))) {
        AcpiReadRsdt(rsdp->rsdtAddress, hw);
    } else {
        hw->cpuCount = 1;
    }

    // -- make sure we do not exceed our capability
    if (hw->cpuCount > MAX_CPU) hw->cpuCount = MAX_CPU;

    ldr_MmuUnmapPage(page);
}


