/****************************************************************************************************************//**
*   @file               lapic.cc
*   @brief              Functions to handle the Local APIC
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-May-05
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
*   | 2021-May-05 | Initial |  v0.0.08 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "boot-interface.h"
#include "kernel-funcs.h"
#include "lapic.h"



/****************************************************************************************************************//**
*   @fn                 Return_t X2ApicInitEarly(BootInterface_t *loaderInterface)
*   @brief              Early initialization function
*
*   Perform the early initialization for the LAPIC.
*
*   @param              loaderInterface     The APIC register in question
*
*   @returns            Whether the module should remain loaded
*
*   @retval             0                   The LAPIC initialization initialization was completed
*   @retval             non-zero            The LAPIC was not able to be initialized and the module can be unloaded
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Return_t X2ApicInitEarly(BootInterface_t *loaderInterface);


/****************************************************************************************************************//**
*   @fn                 int Init(void);
*   @brief              Late initialization function
*
*   Perform the late initialization for the LAPIC.
*
*   @returns            0
*///-----------------------------------------------------------------------------------------------------------------
extern "C" int Init(void);



/****************************************************************************************************************//**
*   @fn                 int ReadApicReg(ApicRegister_t reg)
*   @brief              Read an APIC register
*
*   Read an APIC register and return its value.
*
*   @param              reg                 The register to read; \ref ApicRegister_t
*
*   @returns            The value read from the APIC register or 0 if invalid
*///-----------------------------------------------------------------------------------------------------------------
extern "C" int ReadApicReg(ApicRegister_t reg);



/****************************************************************************************************************//**
*   @fn                 int WriteApicReg(ApicRegister_t reg, uint32_t val)
*   @brief              Write to an APIC register
*
*   Write to  an APIC register and return its value.
*
*   @param              reg                 The register to which to write; \ref ApicRegister_t
*   @param              val                 The value to write to the register
*
*   @returns            0 if invalid register
*///-----------------------------------------------------------------------------------------------------------------
extern "C" int WriteApicReg(ApicRegister_t reg, uint32_t val);



/****************************************************************************************************************//**
*   @fn                 int CheckApicRegStatus(ApicRegister_t reg, uint8_t index);
*   @brief              Read an indexed register
*
*   Read an indexed APIC register and return its value.
*
*   @param              reg                 The register to which to write; \ref ApicRegister_t
*   @param              index               The bit index which to read
*
*   @returns            0 if invalid register
*///-----------------------------------------------------------------------------------------------------------------
extern "C" int CheckApicRegStatus(ApicRegister_t reg, uint8_t index);



#if IS_ENABLED(KERNEL_DEBUGGER) || defined(__DOXYGEN__)

/****************************************************************************************************************//**
*   @fn                 void LapicDebugInit(void)
*   @brief              Perform the Debugger initialization for the LAPIC
*
*   When enabled at compile-time, perform the debugger initialization tasks
*///-----------------------------------------------------------------------------------------------------------------
extern "C" void LapicDebugInit(void);


#endif




/********************************************************************************************************************
*   See documentation in `lapic.h`
*///-----------------------------------------------------------------------------------------------------------------
Apic_t *apic = NULL;



/********************************************************************************************************************
*   See documentation in `lapic.h`
*///-----------------------------------------------------------------------------------------------------------------
void LApicSpurious(Addr_t *regs)
{
}



/********************************************************************************************************************
*   See documentation in `lapic.h`
*///-----------------------------------------------------------------------------------------------------------------
bool IsReadable(ApicRegister_t reg)
{
    switch (reg) {
        case APIC_LOCAL_ID:
        case APIC_LOCAL_VERSION:
        case APIC_TPR:
        case APIC_APR:
        case APIC_PPR:
        case APIC_RRD:
        case APIC_LDR:
        case APIC_DFR:
        case APIC_SIVR:
        case APIC_ESR:
        case APIC_CMCI:
        case APIC_ICR1:
        case APIC_LVT_TIMER:
        case APIC_LVT_THERMAL_SENSOR:
        case APIC_LVT_PERF_COUNTING_REG:
        case APIC_LVT_LINT0:
        case APIC_LVT_LINT1:
        case APIC_LVT_ERROR:
        case APIC_TIMER_ICR:
        case APIC_TIMER_CCR:
        case APIC_TIMER_DCR:
            return true;

        case APIC_ICR2:
            if (apic->version == XAPIC) return true;
            else return false;

        default:
            return false;
    }
}



/********************************************************************************************************************
*   See documentation in `lapic.h`
*///-----------------------------------------------------------------------------------------------------------------
bool IsWritable(ApicRegister_t reg)
{
    switch (reg) {
        case APIC_TPR:
        case APIC_EOI:
        case APIC_SIVR:
        case APIC_ESR:
        case APIC_CMCI:
        case APIC_ICR1:
        case APIC_LVT_TIMER:
        case APIC_LVT_THERMAL_SENSOR:
        case APIC_LVT_PERF_COUNTING_REG:
        case APIC_LVT_LINT0:
        case APIC_LVT_LINT1:
        case APIC_LVT_ERROR:
        case APIC_TIMER_ICR:
        case APIC_TIMER_DCR:
            return true;

        case APIC_ICR2:
        case APIC_SELF_IPI:
            if (apic->version == XAPIC) return true;
            else return false;

        default:
            return false;
    }
}



/********************************************************************************************************************
*   See documentation in `lapic.h`
*///-----------------------------------------------------------------------------------------------------------------
bool IsStatus(ApicRegister_t reg)
{
    if (reg == APIC_ISR_BASE || reg == APIC_TMR_BASE || reg == APIC_IRR_BASE) return true;
    else return false;
}



/********************************************************************************************************************
*   See documentation above
*///-----------------------------------------------------------------------------------------------------------------
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

            apic = &x2apic;
            if (!apic->earlyInit) return -EINVAL;
            return apic->earlyInit(loaderInterface);
        } else {
            KernelPrintf(".... xAPIC\n");

            apic = &xapic;
            if (!apic->earlyInit) return -EINVAL;
            return apic->earlyInit(loaderInterface);
        }
    } else {
        KernelPrintf("FATAL: No APIC Available; HALTING!\n");

        while (true) {
            __asm volatile("hlt");
        }
    }


    return -EINVAL;
}



/********************************************************************************************************************
*   See documentation above
*///-----------------------------------------------------------------------------------------------------------------
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



/********************************************************************************************************************
*   See documentation above
*///-----------------------------------------------------------------------------------------------------------------
int ReadApicReg(ApicRegister_t reg)
{
    if (!apic) return 0;
    if (!apic->readApicRegister) return 0;
    return apic->readApicRegister(reg);
}



/********************************************************************************************************************
*   See documentation above
*///-----------------------------------------------------------------------------------------------------------------
int WriteApicReg(ApicRegister_t reg, uint32_t val)
{
    if (!apic) return -EINVAL;
    if (!apic->writeApicRegister) return -EINVAL;
    apic->writeApicRegister(reg, val);

    return 0;
}



/********************************************************************************************************************
*   See documentation above
*///-----------------------------------------------------------------------------------------------------------------
int CheckApicRegStatus(ApicRegister_t reg, uint8_t index)
{
    if (!apic) return -EINVAL;
    if (!apic->checkIndexedStatus) return -EINVAL;

    if (apic->checkIndexedStatus(reg, index)) return 1;
    else return 0;
}



/****************************************************************************************************************//**
*   @fn                 uint64_t tmr_GetCurrentTimer(void)
*   @brief              Read the current timer count
*
*   @returns            The current timer count
*///-----------------------------------------------------------------------------------------------------------------
extern "C" uint64_t tmr_GetCurrentTimer(void)
{
    return apic->ticker;
}



/****************************************************************************************************************//**
*   @fn                 Return_t tmr_Tick(void)
*   @brief              Called when a timer tick is happens
*
*   Only CPU0 will increment the timer counter
*
*   @returns            The current timer count after updating
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Return_t tmr_Tick(void)
{
    if (unlikely(ThisCpu()->cpuNum == 0)) {
        apic->ticker += 1000;
    }

    return apic->ticker;
}



/****************************************************************************************************************//**
*   @fn                 Return_t tmr_Eoi(void)
*   @brief              Issue an EOI to the LAPIC
*
*   Issue an End Of Interrupt to the LAPIC
*
*   @returns            0
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Return_t tmr_Eoi(void)
{
    apic->writeApicRegister(APIC_EOI, 0);
    return 0;
}



/****************************************************************************************************************//**
*   @fn                 int ipi_LapicGetId(void)
*   @brief              Read the Local APIC ID
*
*   Read the Local APIC ID, equivalent to the CPU number
*
*   @returns            Local APIC ID
*///-----------------------------------------------------------------------------------------------------------------
extern "C" int ipi_LapicGetId(void)
{
    return apic->getId();
}



/****************************************************************************************************************//**
*   @fn                 int ipi_SendInit(int core)
*   @brief              Send an INIT IPI to a core
*
*   Send an INIT IPI to a core
*
*   @param              core                The core to receive the INIT IPI
*
*   @returns            0
*///-----------------------------------------------------------------------------------------------------------------
extern "C" int ipi_SendInit(int core)
{
    // -- Hi bits are xxxx xxxx 0000 0000 0000 0000 0000 0000
    // -- Lo bits are 0000 0000 0000 xx00 xx0x xxxx 0000 0000
    //                               ++   || | |+-+
    //                               |    || | | |
    //                               |    || | | +--------- delivery mode (101)
    //    Destination Shorthand (00) +    || | +----------- destination mode (0)
    //                                    || +------------- delivery status (1)
    //                                    |+--------------- level (1)
    //                                    +---------------- trigger (1)
    //
    //   or 0000 0000 0000 0000 1101 0101 0000 0000 (0x0000d500)

    uint64_t icr = 0x000000000000d500 | (((uint64_t)core & 0xff) << 56);

#if DEBUG_ENABLED(ipi_SendInit)
    char buf[64];
    ksprintf(buf, "XAPIC: Sending INIT!!!\n");
    KernelPrintf(buf);
#endif

    apic->writeApicIcr(icr);

    return 0;
}



/****************************************************************************************************************//**
*   @fn                 int ipi_SendSipi(int core, Addr_t vector)
*   @brief              Send a Startup IPI to a core
*
*   Send an Startup IPI to a core
*
*   @param              core                The core to receive the INIT IPI
*   @param              vector              The segment register (offset 0x0000) to set for the startup location
*
*   @returns            0
*///-----------------------------------------------------------------------------------------------------------------
extern "C" int ipi_SendSipi(int core, Addr_t vector)
{
    // -- Hi bits are xxxx xxxx 0000 0000 0000 0000 0000 0000
    // -- Lo bits are 0000 0000 0000 xx00 xx0x xxxx 0000 0000
    //                               ++   || | |+-+ +-------+
    //                               |    || | | |      +   startup vector (vector >> 12)
    //                               |    || | | +--------- delivery mode (110)
    //    Destination Shorthand (00) +    || | +----------- destination mode (0)
    //                                    || +------------- delivery status (1)
    //                                    |+--------------- level (1)
    //                                    +---------------- trigger (1)
    //
    //   or 0000 0000 0000 0000 1101 0110 0000 0000 (0x0000d600)

    uint64_t icr = 0x000000000000d600 | (((uint64_t)core & 0xff) << 56) | ((vector >> 12) & 0xff);

    apic->writeApicIcr(icr);

    return 0;
}



/****************************************************************************************************************//**
*   @fn                 int ipi_SendIpi(int vector)
*   @brief              Broadcast an Startup IPI to all cores
*
*   Broadcast an Startup IPI to all cores, except the current core
*
*   @param              vector              The interrupt vector to send to the core
*
*   @returns            0
*///-----------------------------------------------------------------------------------------------------------------
extern "C" int ipi_SendIpi(int vector)
{
    // -- Hi bits are 0000 0000 0000 0000 0000 0000 0000 0000
    // -- Lo bits are 0000 0000 0000 xx00 0000 0000 xxxx xxxx
    //                               ++             +-------+
    //                               |                  +-- vector (vector >> 12)
    //                               |
    //    Destination Shorthand (11) +
    //
    //   or 0000 0000 0000 1100 0000 0000 0000 0000 (0x000c0000)

    uint64_t icr = 0x00000000000c0000 | (vector & 0xff);

    apic->writeApicIcr(icr);

    return 0;
}



#if IS_ENABLED(KERNEL_DEBUGGER) || defined(__DOXYGEN__)



/****************************************************************************************************************//**
*   @fn                 void DebugTimerCounts(void)
*   @brief              Debug the timer over all CPUs
*
*   Print the current timer counts
*///-----------------------------------------------------------------------------------------------------------------
void DebugTimerCounts(void)
{
    char buf[100];

    // -- now we have the values -- dump them
    DbgOutput(ANSI_CLEAR ANSI_SET_CURSOR(0,0) ANSI_FG_BLUE ANSI_ATTR_BOLD "Current Timer Count\n");

    ksprintf(buf, "%ld\n", tmr_GetCurrentTimer());
    DbgOutput(buf);
}



/****************************************************************************************************************//**
*   @var                tmrStates
*   @brief              Debugger states for the Timer Debugger Module
*///-----------------------------------------------------------------------------------------------------------------
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



/****************************************************************************************************************//**
*   @var                tmrTrans
*   @brief              Debugger transitions for the Timer Debugger Module
*///-----------------------------------------------------------------------------------------------------------------
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



/****************************************************************************************************************//**
*   @var                tmrModule
*   @brief              Debugger module definitions structure for the Timer Debugger Module
*///-----------------------------------------------------------------------------------------------------------------
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
*   @fn                 void LapicDebugInit(void)
*   @brief              Initialize the debugger module structure
*
*   Initialize the debugger module for the LAPIC
*///-----------------------------------------------------------------------------------------------------------------
extern "C" void LapicDebugInit(void)
{
    tmrModule.stack = KrnStackFind();
    for (Addr_t s = tmrModule.stack; s < tmrModule.stack + STACK_SIZE; s += PAGE_SIZE) {
        MmuMapPage(s, PmmAlloc(), PG_WRT);
    }
    tmrModule.stack += STACK_SIZE;

    DbgRegister(&tmrModule, tmrStates, tmrTrans);
}



#endif