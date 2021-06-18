//===================================================================================================================
//
//  cpu.cc -- Complete the CPU structure initialization
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jun-18  Initial  v0.0.9c  ADCL  Initial version
//
//===================================================================================================================




#include "types.h"
#include "cpu.h"



//
// -- The cpus abstraction structure
//    ------------------------------
ArchCpu_t cpus[MAX_CPU] = { {0} };



//
// -- Initialize the CPU structures to initial values
//    -----------------------------------------------
void CpuInit(void)
{
    for (int i = 0; i < MAX_CPU; i ++) {
        cpus[i].cpuNum = i;
        cpus[i].cpu = &cpus[i];
        cpus[i].process = 0;
    }
}



