//====================================================================================================================
//
//  cpu-debugger.cc -- Debugging functions for the cpu
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Oct-30  Initial  v0.0.12  ADCL  Initial version (copied from century)
//
//===================================================================================================================



#include "types.h"
#include "cpu.h"
#include "kernel-funcs.h"
#include "scheduler.h"
#include "stacks.h"
#include "debugger.h"



#if IS_ENABLED(KERNEL_DEBUGGER)


//
// -- This function will clear the screen and print out the data headings
//    -------------------------------------------------------------------
static void PrintHeadings(void)
{
#define B ANSI_ATTR_BOLD ANSI_FG_BLUE
#define N ANSI_ATTR_NORMAL

    DbgOutput(ANSI_CLEAR ANSI_SET_CURSOR(0,0));
    DbgOutput("+------------------------+\n");
    DbgOutput("| " B "CPU" N "                    |\n");
    DbgOutput("+------------------------+\n");
    DbgOutput("| " B "Process Address:" N "       |\n");
    DbgOutput("| " B "Process ID:" N "            |\n");
    DbgOutput("| " B "Command:" N "               |\n");
    DbgOutput("| " B "Virtual Address Space:" N " |\n");
    DbgOutput("| " B "Base Stack Frame:" N "      |\n");
    DbgOutput("| " B "Status:" N "                |\n");
    DbgOutput("| " B "Priority:" N "              |\n");
    DbgOutput("| " B "Quantum Left:" N "          |\n");
    DbgOutput("| " B "Time Used:" N "             |\n");
    DbgOutput("| " B "Wake At:" N "               |\n");
    DbgOutput("| " B "Queue Status:" N "          |\n");
    DbgOutput("+------------------------+\n");

#undef B
#undef N
}


//
// -- Output the interesting values for a CPU
//    ---------------------------------------
void PrintProcess(int cpu, volatile Process_t *proc)
{
    if (!proc) return;
    char buf[64];

    int fwd = (cpu * 19) + 25;

    DbgOutput(ANSI_SET_CURSOR(0,0));

    ksprintf(buf, "\x1b[%dC+------------------+\n", fwd);
    DbgOutput(buf);

    ksprintf(buf, "\x1b[%dC|        " ANSI_ATTR_BOLD "CPU%d" ANSI_ATTR_NORMAL "      |\n", fwd, cpu);
    DbgOutput(buf);

    ksprintf(buf, "\x1b[%dC+------------------+\n", fwd);
    DbgOutput(buf);

    ksprintf(buf, "\x1b[%dC| %p |\n", fwd, proc);
    DbgOutput(buf);


    if (proc) {
        ksprintf(buf, "\x1b[%dC| %16d |\n", fwd, proc->pid);
        DbgOutput(buf);

        ksprintf(buf, "\x1b[%dC| %-16.16s |\n", fwd, proc->command);
        DbgOutput(buf);

        ksprintf(buf, "\x1b[%dC| %p |\n", fwd, proc->virtAddrSpace);
        DbgOutput(buf);

        ksprintf(buf, "\x1b[%dC| %p |\n", fwd, 0);
        DbgOutput(buf);

        ksprintf(buf, "\x1b[%dC| %-16.16s |\n", fwd, ProcStatusStr(proc->status));
        DbgOutput(buf);

        ksprintf(buf, "\x1b[%dC| %-16.16s |\n", fwd, ProcPriorityStr(proc->priority));
        DbgOutput(buf);

        ksprintf(buf, "\x1b[%dC| %16d |\n", fwd, AtomicRead(&proc->quantumLeft));
        DbgOutput(buf);

        ksprintf(buf, "\x1b[%dC| %16ld |\n", fwd, proc->timeUsed);
        DbgOutput(buf);

        ksprintf(buf, "\x1b[%dC| %16ld |\n", fwd, proc->wakeAtMicros);
        DbgOutput(buf);


        if (proc->stsQueue.next == &proc->stsQueue) {
            ksprintf(buf, "\x1b[%dC| " ANSI_FG_GREEN ANSI_ATTR_BOLD "Not on a queue" ANSI_ATTR_NORMAL "   |\n", fwd);
            DbgOutput(buf);

        } else {
            ksprintf(buf, "\x1b[%dC| " ANSI_FG_RED ANSI_ATTR_BOLD "On some queue" ANSI_ATTR_NORMAL "    |\n", fwd);
            DbgOutput(buf);

        }
    }

    ksprintf(buf, "\x1b[%dC+------------------+\n", fwd);
    DbgOutput(buf);
}


//
// -- Dump the interesting values from the running processes on each CPU
//    ------------------------------------------------------------------
void DebugSchedulerRunning(void)
{
    PrintHeadings();
    for (int i = 0; i < KrnActiveCores(); i ++) {
        PrintProcess(i, cpus[i].process);
    }
}



//
// -- Dump the contents of the register
//    ---------------------------------
void DebugRegisterDump(void)
{
    char strCpu[20] = {0};

    DbgPromptGeneric("<cpu>", strCpu, sizeof(strCpu));

    int cpu = 0;
    char *s = strCpu;

    while (*s) {
        if (*s >= '0' && *s <= '9') {
            cpu = cpu * 10 + *s - '0';
        } else {
            DbgOutput(ANSI_ERASE_LINE ANSI_FG_RED "invalid cpu number\n");
            return;
        }

        s ++;
    }

    if (cpu < 0 || cpu >= cpusActive) {
        DbgOutput(ANSI_ERASE_LINE ANSI_FG_RED "invalid cpu number\n");
        return;
    }

    if (cpu == LapicGetId()) {
        DbgOutput(ANSI_ERASE_LINE ANSI_FG_RED "cannot dump registers for current cpu\n");
        return;
    }

    Addr_t *regs = (Addr_t *)cpus[cpu].stackTop;
    char buf[60];
    const char *name[] = {
        "GS",
        "FS",
        "ES",
        "DS",
        "CR4",
        "CR3",
        "CR2",
        "CR0",
        "R15",
        "R14",
        "R13",
        "R12",
        "R11",
        "R10",
        "R9",
        "R8",
        "RDI",
        "RSI",
        "RBP",
        "RDX",
        "RCX",
        "Return",
        "RBX",
        "RAX",
        "INT",
        "ERR",
        "RIP",
        "CS",
        "RFLAGS",
        "RSP",
        "SS",
    };

    DbgOutput(ANSI_CLEAR ANSI_SET_CURSOR(0,0));

    ksprintf(buf, "Dumping Registers for CPU%d\n", cpu);
    DbgOutput(buf);

    for (int i = 0; i < 31; i ++) {
        ksprintf(buf, "| %-10.10s | %p |\n", name[i], regs[i]);
        DbgOutput(buf);
    }
}



//
// -- here is the debugger menu & function ecosystem
//    ----------------------------------------------
DbgState_t cpuStates[] = {
    {   // -- state 0
        .name = "cpu",
        .transitionFrom = 0,
        .transitionTo = 2,
    },
    {   // -- state 1 (status)
        .name = "status",
        .function = (Addr_t)DebugSchedulerRunning,
    },
    {   // -- state 2 (registers)
        .name = "regs",
        .function = (Addr_t)DebugRegisterDump,
    },
};


DbgTransition_t cpuTrans[] = {
    {   // -- transition 0
        .command = "status",
        .alias = "s",
        .nextState = 1,
    },
    {   // -- transition 1
        .command = "regs",
        .alias = "r",
        .nextState = 2,
    },
    {   // -- transition 2
        .command = "exit",
        .alias = "x",
        .nextState = -1,
    },
};


DbgModule_t cpuModule = {
    .name = "cpu",
    .addrSpace = GetAddressSpace(),
    .stack = 0,     // -- needs to be handled during late init
    .stateCnt = sizeof(cpuStates) / sizeof (DbgState_t),
    .transitionCnt = sizeof(cpuTrans) / sizeof (DbgTransition_t),
    .list = {&cpuModule.list, &cpuModule.list},
    .lock = {0},
    // -- it does not matter what we put for .states and .transitions; will be replaced in debugger
};


/****************************************************************************************************************//**
*   @fn                 void PmmDebugInit(void)
*   @brief              Initialize the debugger module structure
*
*   Initialize the debugger module for the PMM
*///-----------------------------------------------------------------------------------------------------------------
extern "C" void CpuDebugInit(void)
{
    extern Addr_t __stackSize;

    cpuModule.stack = StackFind();
    for (Addr_t s = cpuModule.stack; s < cpuModule.stack + __stackSize; s += PAGE_SIZE) {
        MmuMapPage(s, PmmAlloc(), PG_WRT);
    }
    cpuModule.stack += __stackSize;

    DbgRegister(&cpuModule, cpuStates, cpuTrans);
}




#endif