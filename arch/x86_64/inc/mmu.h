//===================================================================================================================
//
//  mmu.h -- Functions related to managing the Paging Tables
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Feb-16  Initial  v0.0.6   ADCL  Initial version
//
//===================================================================================================================


#pragma once

#include "types.h"



/****************************************************************************************************************//**
*   @addtogroup         PG      Paging Flags Utilized by the Loader
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
#define PG_NONE     (0)
/****************************************************************************************************************//**
*   @def                PG_WRT
*   @brief              The page is writable.
*
*   Absence of this flag indicates that the page is ready only.
*///-----------------------------------------------------------------------------------------------------------------
#define PG_WRT      (1<<0)
/****************************************************************************************************************//**
*   @def                PG_KRN
*   @brief              This is a kernel page.
*
*   This is a kernel page.  This sets a flag on the page structure so that the page is not allowed to be swapped
*   out by paging.
*
*   @note               Page swapping is not yet implemented.
*///-----------------------------------------------------------------------------------------------------------------
#define PG_KRN      (1<<1)
/****************************************************************************************************************//**
*   @def                PG_DEV
*   @brief              This is a Memory Mapped IO page.
*
*   This flag will also disable cache for the page.
*///-----------------------------------------------------------------------------------------------------------------
#define PG_DEV      (1<<15)

/// @}


#include "mmu-module.h"

