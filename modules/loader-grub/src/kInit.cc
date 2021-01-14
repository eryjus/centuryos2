//===================================================================================================================
//
//  kInit.cc -- Complete the initialization
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-03  Initial  v0.0.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "elf.h"
#include "mboot.h"
#include "serial.h"


extern "C" {
    void kInit(void);
    void JumpKernel(Addr_t entry);
}


//
// -- Complete the initialization
//    ---------------------------
void kInit(void)
{
    SerialOpen();
    SerialPutString("Hello\n");

    Addr_t kernel = MBootGetKernel();

    if (kernel != 0) {
        Addr_t entry = ElfLoadKernel(kernel);

//        SerialPutString("Jumping!\n");

        JumpKernel(entry);
    }

    while (true) {}
}

