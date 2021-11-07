/****************************************************************************************************************//**
*   @file               mmu-loader.cc
*   @brief              Loader-specific functions related to managing the Paging Tables
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



#include "types.h"
#include "serial.h"
#include "mmu.h"



/********************************************************************************************************************
*   Documented in `mmu-funcs.h`
*///-----------------------------------------------------------------------------------------------------------------
void ldr_MmuEmptyPdpt(int index)
{
    extern Frame_t earlyFrame;

#if DEBUG_ENABLED(cmn_MmuEmptyPdpt)

        SerialPutString("Creating a PDPT table for PML4 index: ");
        SerialPutHex64(index);
        SerialPutChar('\n');

#endif

    PageEntry_t *ent = &((PageEntry_t *)0xfffffffffffff000)[index];

    if (!ent->p) {
        ent->frame = earlyFrame ++;

#if DEBUG_ENABLED(cmn_MmuEmptyPdpt)

        SerialPutString("Next frame is: ");
        SerialPutHex64(ent->frame);
        SerialPutChar('\n');

#endif

        ent->rw = 1;
        ent->p = 1;

        uint64_t *tbl = (uint64_t *)(0xffffffffffe00000 | (index << 12));
        INVLPG((Addr_t)tbl);

#if DEBUG_ENABLED(cmn_MmuEmptyPdpt)

        SerialPutString("Clearing the table starting at: ");
        SerialPutHex64((Addr_t)tbl);
        SerialPutChar('\n');

#endif

        for (int i = 0; i < 512; i ++) {
#if DEBUG_ENABLED(cmn_MmuEmptyPdpt)
            SerialPutString("At address: ");
            SerialPutHex64((Addr_t)&tbl[i]);
            SerialPutChar('\n');

#endif
            tbl[i] = 0;
        }
    }
}


