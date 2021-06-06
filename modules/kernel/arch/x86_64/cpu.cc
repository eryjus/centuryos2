

#include "types.h"
#include "cpu.h"



ArchCpu_t cpus[MAX_CPU] = {{0}};



void CpuInit(void)
{
    for (int i = 0; i < MAX_CPU; i ++) {
        cpus[i].cpuNum = i;
    }
}



