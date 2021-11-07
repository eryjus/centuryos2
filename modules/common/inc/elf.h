/****************************************************************************************************************//**
*   @file               elf.h
*   @brief              Functions related to parsing an elf executable
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Jan-04
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
*   | 2021-Jan-04 | Initial |  v0.0.01 | ADCL | Initial version
*
*///=================================================================================================================



#pragma once

#include "types.h"



/****************************************************************************************************************//**
*   @fn                 Addr_t ElfLoadImage(Addr_t location)
*   @brief              Load an ELF Image
*
*   Load an ELF image at a location and prepare it for execution
*
*   @param              location        The address of the ELF header to evaluate
*
*   @returns            The stated entry point for the ELF executable
*
*   @retval             NULL            When the ELF could not be loaded
*   @retval             non-zero        The ELF was loaded and this is the entry address
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Addr_t ElfLoadImage(Addr_t location);


