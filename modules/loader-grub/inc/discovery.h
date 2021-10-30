/****************************************************************************************************************//**
*   @file               discovery.h
*   @brief              Platform-specific hardware discovery
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Oct-24
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
*   | 2021-Oct-24 | Initial |  v0.0.12 | ADCL | Initial version
*
*///=================================================================================================================



#pragma once

#include "types.h"
#include "boot-interface.h"



/****************************************************************************************************************//**
*   @fn                 void PlatformDiscovery(BootInterface_t *hw)
*   @brief              Perform any platform-specific discovery and populate the boot information structure
*
*   Perform any platform-specific hardware discovery.
*///-----------------------------------------------------------------------------------------------------------------
extern "C" void PlatformDiscovery(BootInterface_t *hw);

