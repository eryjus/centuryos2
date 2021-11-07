/****************************************************************************************************************//**
*   @file               mmu-arch.h
*   @brief              Functions and structures for managing the x86_64 MMU
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Nov-07
*   @since              v0.0.13
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Nov-07 | Initial |  v0.0.13 | ADCL | Initial version
*
*///=================================================================================================================


#ifndef __MMU_H__
# error "Do not inclide 'mmu-arch.h' directly; include 'mmu.h' instead"
#endif



#pragma once



/****************************************************************************************************************//**
*   @typedef            PageEntry_t
*   @brief              Formalization of the \ref PageEntry_t structure into a defined type
*///----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             PageEntry_t
*   @brief              The structure of the x86_64-pc page table Entry
*
*   This structure is dictated by the x86_64 CPU architecture.  It has several bit fields which are directly
*   analogous to the bit fields in the MMU itself, so there is a direect translaction from C to the hardware.
*   This is a 64-bit Entry and this 1 structure represents all levels of the page tables.
*///----------------------------------------------------------------------------------------------------------------
typedef struct PageEntry_t {
    unsigned int p : 1;                 //!< Is the page present?
    unsigned int rw : 1;                //!< set to 1 to allow writes
    unsigned int us : 1;                //!< 0=Supervisor; 1=user
    unsigned int pwt : 1;               //!< Page Write Through
    unsigned int pcd : 1;               //!< Page-level cache disable
    unsigned int a : 1;                 //!< accessed
    unsigned int d : 1;                 //!< dirty (needs to be written for a swap)
    unsigned int pat : 1;               //!< set to 0 for tables, page Page Attribute Table (set to 0)
    unsigned int g : 1;                 //!< Global (set to 0)
    unsigned int k : 1;                 //!< Is this a kernel page?
    unsigned int avl : 2;               //!< Available for software use
    Frame_t frame : 36;                 //!< This is the 4K aligned page frame address (or table address)
    unsigned int reserved : 4;          //!< reserved bits
    unsigned int software : 11;         //!< software use bits
    unsigned int xd : 1;                //!< execute disable
} __attribute__((packed)) PageEntry_t;



/****************************************************************************************************************//**
*   @fn                 PageEntry_t *GetPML4Entry(Addr_t a);
*   @brief              Obtain the PML4 Entry for an address
*
*   Obtain the PML4 Entry for an address in virtual address space.
*
*   @param              a               The address for which there is interest
*
*   @returns            A pointer to the PML4 Entry
*
*   @note               This function operates in the current address space.
*
*   @note               The PML4 Entry is guaranteed to exist and be accessible in memory.  However, the PML4 Entry
*                       is not guaranteed to contain a valid mapping.
*///-----------------------------------------------------------------------------------------------------------------
extern "C" PageEntry_t *GetPML4Entry(Addr_t a);



/****************************************************************************************************************//**
*   @fn                 PageEntry_t *GetPDPTEntry(Addr_t a);
*   @brief              Obtain the PDPT Entry for an address
*
*   Obtain the PDPT Entry for an address in virtual address space.
*
*   @param              a               The address for which there is interest
*
*   @returns            A pointer to the PDPT Entry
*
*   @note               This function operates in the current address space.
*
*   @note               The PDPT Entry is not guaranteed to exist and not guaranteed to be accessible in memory.
*                       The PML4 must be checked to be certain.  This function makes no protections and a `#PF`
*                       may occur.
*///-----------------------------------------------------------------------------------------------------------------
extern "C" PageEntry_t *GetPDPTEntry(Addr_t a);



/****************************************************************************************************************//**
*   @fn                 PageEntry_t *GetPDEntry(Addr_t a);
*   @brief              Obtain the PD Entry for an address
*
*   Obtain the PD Entry for an address in virtual address space.
*
*   @param              a               The address for which there is interest
*
*   @returns            A pointer to the PD Entry
*
*   @note               This function operates in the current address space.
*
*   @note               The PD Entry is not guaranteed to exist and not guaranteed to be accessible in memory.
*                       The PDPT must be checked to be certain.  This function makes no protections and a `#PF`
*                       may occur.
*///-----------------------------------------------------------------------------------------------------------------
extern "C" PageEntry_t *GetPDEntry(Addr_t a);



/****************************************************************************************************************//**
*   @fn                 PageEntry_t *GetPTEntry(Addr_t a);
*   @brief              Obtain the PT Entry for an address
*
*   Obtain the PT Entry for an address in virtual address space.
*
*   @param              a               The address for which there is interest
*
*   @returns            A pointer to the PT Entry
*
*   @note               This function operates in the current address space.
*
*   @note               The PT Entry is not guaranteed to exist and not guaranteed to be accessible in memory.
*                       The PD must be checked to be certain.  This function makes no protections and a `#PF`
*                       may occur.
*///-----------------------------------------------------------------------------------------------------------------
extern "C" PageEntry_t *GetPTEntry(Addr_t a);


