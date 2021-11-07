//===================================================================================================================
//
//  lapic-init.cc -- Functions to handle the lapic initialization
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-May-05  Initial  v0.0.8   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "boot-interface.h"
#include "kernel-funcs.h"
#include "lapic.h"


//
// -- Function prototypes
//    -------------------
extern "C" {
    Return_t X2ApicInitEarly(BootInterface_t *loaderInterface);
    int Init(void);
    int ReadApicReg(ApicRegister_t reg);
    int WriteApicReg(ApicRegister_t reg, uint32_t val);
    int CheckApicRegStatus(ApicRegister_t reg, uint8_t index);

#if IS_ENABLED(KERNEL_DEBUGGER)

    void LapicDebugInit(void);

#endif
}


//
// -- global variables
//    ----------------
Apic_t *apic = NULL;


//
// -- Initialize the x2APIC, cascading down to the 8259 PIC if required
//    -----------------------------------------------------------------
Return_t X2ApicInitEarly(BootInterface_t *loaderInterface)
{
    ProcessInitTable();

    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    KernelPrintf("Initializing the Local APIC\n");


    // -- First check for xAPIC support
    CPUID(1, &eax, &ebx, &ecx, &edx);
    if (edx & CPUID_FEAT_EDX_APIC) {
        KernelPrintf(".. there is some form of APIC\n");

        // -- Check for an x2APIC
        if (ecx & CPUID_FEAT_ECX_X2APIC) {
            KernelPrintf(".... x2APIC\n");

 #ifdef __X2APIC__
            apic = &x2apic;
            if (!apic->earlyInit) return -EINVAL;
            return apic->earlyInit(loaderInterface);
 #else
            return -EINVAL;
 #endif
        } else {
            KernelPrintf(".... xAPIC\n");

#ifdef __XAPIC__
            apic = &xapic;
            if (!apic->earlyInit) return -EINVAL;
            return apic->earlyInit(loaderInterface);
#else
            return -EINVAL;
#endif
        }
    } else {
        KernelPrintf("FATAL: No APIC Available; HALTING!\n");

        while (true) {
            __asm volatile("hlt");
        }
    }


    return -EINVAL;
}


//
// -- End of Interrupt signal
//    -----------------------
int Eoi(void)
{
    if (!apic) return -EINVAL;
    if (!apic->eoi) return -EINVAL;
    apic->eoi();

    return 0;
}


//
// -- Late initialization function
//    ----------------------------
int Init(void)
{
    ProcessInitTable();
    KernelPrintf("Performing the LAPIC late initialization\n");

#if IS_ENABLED(KERNEL_DEBUGGER)

    LapicDebugInit();

#endif

    if (!apic) return -EINVAL;
    if (apic->init) apic->init();

    return 0;
}


//
// -- Read an APIC register
//    ---------------------
int ReadApicReg(ApicRegister_t reg)
{
    if (!apic) return 0;
    if (!apic->readApicRegister) return 0;
    return apic->readApicRegister(reg);
}


//
// -- Write an APIC register
//    ----------------------
int WriteApicReg(ApicRegister_t reg, uint32_t val)
{
    if (!apic) return -EINVAL;
    if (!apic->writeApicRegister) return -EINVAL;
    apic->writeApicRegister(reg, val);

    return 0;
}


//
// -- Check APIC status
//    -----------------
int CheckApicRegStatus(ApicRegister_t reg, uint8_t index)
{
    if (!apic) return -EINVAL;
    if (!apic->checkIndexedStatus) return -EINVAL;

    if (apic->checkIndexedStatus(reg, index)) return 1;
    else return 0;
}



//
// -- Get the current tick count
//    --------------------------
extern "C" uint64_t tmr_GetCurrentTimer(void);
uint64_t tmr_GetCurrentTimer(void)
{
    return apic->currentTimer();
}


//
// -- Handle a timer tick
//    -------------------
extern "C" Return_t tmr_Tick(void);
Return_t tmr_Tick(void)
{
    if (unlikely(ThisCpu()->cpuNum == 0) && likely(apic->tick != NULL)) apic->tick();
    return apic->currentTimer();
}


//
// -- Handle an EOI for this LAPIC
//    ----------------------------
extern "C" Return_t tmr_Eoi(void);
Return_t tmr_Eoi(void)
{
    apic->eoi();
    return 0;
}


//
// -- Get the current CPU Id
//    ----------------------
extern "C" int ipi_LapicGetId(void)
{
    return apic->getId();
}



//
// -- Send the Init IPI to a core
//    ---------------------------
extern "C" int ipi_SendInit(int core)
{
    return apic->sendInit(core);
}



//
// -- Send the SIPI to a core
//    -----------------------
extern "C" int ipi_SendSipi(int core, Addr_t vector)
{
    return apic->sendSipi(core, vector);
}



//
// -- Broadcast an IPI
//    ----------------
extern "C" int ipi_SendIpi(int vector)
{
    return apic->sendIpi(vector);
}




#if IS_ENABLED(KERNEL_DEBUGGER)


#include "stacks.h"


//
// -- Debug the timer over all CPUs
//    -----------------------------
void DebugTimerCounts(void)
{
    char buf[100];

    // -- now we have the values -- dump them
    DbgOutput(ANSI_CLEAR ANSI_SET_CURSOR(0,0) ANSI_FG_BLUE ANSI_ATTR_BOLD "Current Timer Count\n");

    ksprintf(buf, "%ld\n", tmr_GetCurrentTimer());
    DbgOutput(buf);
}



//
// -- here is the debugger menu & function ecosystem
//    ----------------------------------------------
DbgState_t tmrStates[] = {
    {   // -- state 0
        .name = "timer",
        .transitionFrom = 0,
        .transitionTo = 1,
    },
    {   // -- state 1 (counts)
        .name = "count",
        .function = (Addr_t)DebugTimerCounts,
    },
};


DbgTransition_t tmrTrans[] = {
    {   // -- transition 0
        .command = "count",
        .alias = "c",
        .nextState = 1,
    },
    {   // -- transition 1
        .command = "exit",
        .alias = "x",
        .nextState = -1,
    },
};


DbgModule_t tmrModule = {
    .name = "timer",
    .addrSpace = GetAddressSpace(),
    .stack = 0,     // -- needs to be handled during late init
    .stateCnt = sizeof(tmrStates) / sizeof (DbgState_t),
    .transitionCnt = sizeof(tmrTrans) / sizeof (DbgTransition_t),
    .list = {&tmrModule.list, &tmrModule.list},
    .lock = {0},
    // -- it does not matter what we put for .states and .transitions; will be replaced in debugger
};


/****************************************************************************************************************//**
*   @fn                 void PmmDebugInit(void)
*   @brief              Initialize the debugger module structure
*
*   Initialize the debugger module for the PMM
*///-----------------------------------------------------------------------------------------------------------------
extern "C" void LapicDebugInit(void)
{
    extern Addr_t __stackSize;

    tmrModule.stack = StackFind();
    for (Addr_t s = tmrModule.stack; s < tmrModule.stack + __stackSize; s += PAGE_SIZE) {
        MmuMapPage(s, PmmAlloc(), PG_WRT);
    }
    tmrModule.stack += __stackSize;

    DbgRegister(&tmrModule, tmrStates, tmrTrans);
}






#endif