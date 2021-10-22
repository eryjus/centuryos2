/****************************************************************************************************************//**
*   @file               mboot.h
*   @brief              Functions related to parsing the multiboot info
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Jan-03
*   @since              v0.0.01
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Jan-03 | Initial |  v0.0.01 | ADCL | Initial version
*
*///=================================================================================================================



#pragma once

#include "types.h"


/****************************************************************************************************************//**
*   @fn                 Addr_t MBootGetKernel(void)
*   @brief              Read the Multiboot Information structure and get the kernel's location in physical memory
*
*   @returns            The address in physical memory of the kernel module
*
*   @retval             address     The kernel module's address
*   @retval             0           When the kernel is not found
*
*   Depending on how this loader was booted, call the proper function to parse the Multiboot Information.  Locate
*   the kernel module and return its physical address.
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Addr_t MBootGetKernel(void);

