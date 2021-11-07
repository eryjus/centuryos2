/****************************************************************************************************************//**
*   @file               mmu-funcs.h
*   @brief              Functions related to managing the Paging Tables
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
# error "Do not inclide 'mmu-funcs.h' directly; include 'mmu.h' instead"
#endif



#pragma once



#include "types.h"



/****************************************************************************************************************//**
*   @fn                 Return_t cmn_MmuMapPage(Addr_t a, Frame_t f, int flags)
*   @brief              Map an address to a physical frame
*
*   In the current address space, map an address to the frame provided.
*
*   @param              a               The address to map
*   @param              f               The frame to support the mapped address
*   @param              flags           Flags indicating how this mapping will be used; see \ref PG
*
*   @returns            0
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Return_t cmn_MmuMapPage(Addr_t a, Frame_t f, int flags);


/****************************************************************************************************************//**
*   @fn                 Return_t cmn_MmuUnmapPage(Addr_t a)
*   @brief              Unmap an address from its physical frame
*
*   In the current address space, unmap an address, releasing its frame.
*
*   @param              a               The address to unmap
*
*   @returns            0
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Return_t cmn_MmuUnmapPage(Addr_t a);


/****************************************************************************************************************//**
*   @fn                 Return_t cmn_MmuIsMapped(Addr_t a)
*   @brief              Determine if an address is mapped
*
*   In the current address space, determine if an address is mapped
*
*   @param              a               The address to check
*
*   @returns            Whether the address is mapped or not
*
*   @retval             0               The address is not mapped
*   @retval             non-zero        The address is mapped
*
*   @note               This function is safe to use in all instances as it will check all the tables top to bottom
*                       in turn.
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Return_t cmn_MmuIsMapped(Addr_t a);



#if defined( __LOADER__) || defined(__DOXYGEN__)



/****************************************************************************************************************//**
*   @fn                 void ldr_MmuEmptyPdpt(int index)
*   @brief              Allocates and clears an empty PDPT table
*
*   Used only in the loader, this function is used to allocate an empty PDPT table to re-populate those entries in
*   the PML4 table.  This is useful for copying the kernel space from the startup environment to all others and
*   guaranteeing that the copy targets get all the additional future address mappings.
*
*   @param              index               The PML4 entry that requires a PDPT table.
*
*   @note               This function is only available in the loader.
*///-----------------------------------------------------------------------------------------------------------------
extern "C" void ldr_MmuEmptyPdpt(int index);



#endif



#if !defined(__LOADER__) || defined(__DOXYGEN__)



/****************************************************************************************************************//**
*   @fn                 Return_t krn_MmuDump(Addr_t addr)
*   @brief              Verbosely dump each level of the paging tables for an address
*
*   In the current address space, dump each level of the paging tables for an address.  This function will dump
*   the able structure until it finds a table which is not mapped.
*
*   @param              addr            The address to check
*
*   @returns            0
*
*   @note               This function is safe to use in all instances as it will check all the tables top to bottom
*                       in turn.
*   @note               This function is not available in the laoder.
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Return_t krn_MmuDump(Addr_t addr);



/****************************************************************************************************************//**
*   @fn                 Return_t krn_MmuMapPageEx(Addr_t space, Addr_t a, Frame_t f, int flags)
*   @brief              In another address space, map an address to a physical frame
*
*   In the the specified address space, map an address to the frame provided.
*
*   @param              space           The address space in which to complete the mapping
*   @param              a               The address to map
*   @param              f               The frame to support the mapped address
*   @param              flags           Flags indicating how this mapping will be used; see \ref PG
*
*   @returns            0
*
*   @note               This function will flush the TLB twice during its execution.  Care is recommended to only
*                       execute this function when address spaces are crossed.
*
*   @see                Return_t cmn_MmuMapPage(Addr_t a, Frame_t f, int flags)
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Return_t krn_MmuMapPageEx(Addr_t space, Addr_t a, Frame_t f, int flags);



/****************************************************************************************************************//**
*   @fn                 Return_t krn_MmuUnmapEx(Addr_t space, Addr_t a)
*   @brief              Unmap an address from its physical frame in another address space
*
*   In another address space, unmap an address, releasing its frame.
*
*   @param              space           The address space in which to unmap
*   @param              a               The address to unmap
*
*   @returns            0
*
*   @note               This function will flush the TLB twice during its execution.  Care is recommended to only
*                       execute this function when address spaces are crossed.
*
*   @see                Return_t cmn_MmuUnmapPage(Addr_t a)
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Return_t krn_MmuUnmapEx(Addr_t space, Addr_t a);


#endif


