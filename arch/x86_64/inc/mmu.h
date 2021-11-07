/****************************************************************************************************************//**
*   @file               mmu.h
*   @brief              Functions related to managing the Paging Tables
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Feb-16
*   @since              v0.0.06
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Feb-16 | Initial |  v0.0.06 | ADCL | Initial version
*
*///=================================================================================================================


#pragma once


/// \cond __DOXYGEN__
#ifndef __MMU_H__
#define __MMU_H__
#endif
/// \endcond


#include "types.h"
#include "mmu-arch.h"
#include "mmu-funcs.h"



/****************************************************************************************************************//**
*   @addtogroup         PG      Paging Flags
*
*   These flags are used in the loader for mapping the kernel into memory.  The kernel will have a more robust
*   implementation of the MMU.  The loader needs a bootstrap version to get things up and running.
*
*   @{
*///-----------------------------------------------------------------------------------------------------------------

/****************************************************************************************************************//**
*   @def                PG_NONE
*   @brief              No paging flags are set.
*
*   No flags are set.  This will imply that the page is read only.
*///-----------------------------------------------------------------------------------------------------------------
#ifndef PG_NONE
#define PG_NONE     (0)
#endif
/****************************************************************************************************************//**
*   @def                PG_WRT
*   @brief              The page is writable.
*
*   Absence of this flag indicates that the page is ready only.
*///-----------------------------------------------------------------------------------------------------------------
#ifndef PG_WRT
#define PG_WRT      (1<<0)
#endif
/****************************************************************************************************************//**
*   @def                PG_KRN
*   @brief              This is a kernel page.
*
*   This is a kernel page.  This sets a flag on the page structure so that the page is not allowed to be swapped
*   out by paging.
*
*   @note               Page swapping is not yet implemented.
*///-----------------------------------------------------------------------------------------------------------------
#ifndef PG_KRN
#define PG_KRN      (1<<1)
#endif
/****************************************************************************************************************//**
*   @def                PG_DEV
*   @brief              This is a Memory Mapped IO page.
*
*   This flag will also disable cache for the page.
*///-----------------------------------------------------------------------------------------------------------------
#ifndef PG_DEV
#define PG_DEV      (1<<15)
#endif

/// @}


#include "mmu-funcs.h"

