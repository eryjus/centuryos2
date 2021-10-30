/****************************************************************************************************************//**
*   @file               lInit.cc
*   @brief              The loader entry point into the C code; already in 64-bit long mode
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Jan-03
*   @since              v0.0.01
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*   This file contains the code used to complete the loader initialization after the CPU is in 64-bit long mode.
*   64-bit long mode requires that paging is enabled and protection is enabled.  Therefore, once the functions
*   in this file get control, the CPU is already in the native settings.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Jan-03 | Initial |  v0.0.01 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "elf-func.h"
#include "mboot.h"
#include "serial.h"
#include "mmu.h"
#include "boot-interface.h"
#include "discovery.h"



//*******************************************************************************************************************
//  Local Function prototypes
//-------------------------------------------------------------------------------------------------------------------
extern "C" void lInit(void);



/****************************************************************************************************************//**
*   @fn                 void JumpKernel(Addr_t entry, Addr_t stack)
*   @brief              Perform a long jump into the kernel module.
*
*   Set the stack for the kernel and then perform a long jump into the kernel entry code.  Note that the current
*   stack is lost.
*
*   @note               This function is implemented in assembly.
*///-----------------------------------------------------------------------------------------------------------------
extern "C" void JumpKernel(Addr_t entry, Addr_t stack);



/****************************************************************************************************************//**
*   @fn                 void lInit(void)
*   @brief              Complete the initialization for the kernel
*
*   Completes all the initialization tasks for the kernel and hands control to the kernel proper.  The primary
*   purpose of this function is to ensure that hardware discovery is complete and that the system is in a state
*   where the kernel can take over.
*///-----------------------------------------------------------------------------------------------------------------
void lInit(void)
{
    extern BootInterface_t *kernelInterface;
    extern Frame_t earlyFrame;
    extern Addr_t pml4;
    extern Addr_t gdtr64;
    const Addr_t interfaceLocation = 0xffff9ffffffff000;

    // -- create all the kernel PML4 entries
    SerialOpen();
    SerialPutString("Hello\n");

#if DEBUG_ENABLED(lInit)

    SerialPutString("Populating all PDPT tables in kernel space\n");

#endif

    for (int i = 0x100; i < 0x1ff; i ++) {
        if ((i >= 0x100 && i < 0x140) || (i >= 0x1f0 && i < 0x1ff)) {
            MmuEmptyPdpt(i);
        }
    }

#if DEBUG_ENABLED(lInit)

    SerialPutString("Mapping the kernel interface structure location\n");

#endif

    Frame_t fr = earlyFrame ++;
    MmuMapPage(interfaceLocation, fr, PG_WRT);
    kernelInterface = (BootInterface_t *)interfaceLocation;
    kernelInterface->modCount = 0;
    kernelInterface->bootVirtAddrSpace = pml4;

    PlatformDiscovery(kernelInterface);

    for (int i = 0; i < MAX_MEM; i ++) {
        kernelInterface->memBlocks[i].start = kernelInterface->memBlocks[i].end = 0;
    }

#if DEBUG_ENABLED(lInit)

    SerialPutString("Getting the kernel\n");
    SerialPutString("  cr3 = ");
    SerialPutHex64(GetCr3());
    SerialPutChar('\n');

#endif

    Addr_t kernel = MBootGetKernel();
    Addr_t stack = (earlyFrame);
    earlyFrame += 4;

#if DEBUG_ENABLED(lInit)

    SerialPutString("Mapping the stack\n");

#endif

    MmuMapPage(0xfffff80000000000, stack + 0, PG_WRT);
    MmuMapPage(0xfffff80000001000, stack + 1, PG_WRT);
    MmuMapPage(0xfffff80000002000, stack + 2, PG_WRT);
    MmuMapPage(0xfffff80000003000, stack + 3, PG_WRT);

#if DEBUG_ENABLED(lInit)

    SerialPutString("Stack mapped\n");

#endif

    if (kernel != 0) {
        Addr_t entry = ElfLoadImage(kernel);

        kernelInterface->nextEarlyFrame = earlyFrame;

#if DEBUG_ENABLED(lInit)

        SerialPutString("Jumping!\n");

#endif

        JumpKernel(entry, STACK_LOCATION + STACK_SIZE);
    }

    while (true) {}
}

