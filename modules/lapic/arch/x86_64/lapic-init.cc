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
// -- Handle a timer IRQ
//    ------------------
extern "C" void tmr_Interrupt(int, Addr_t *reg);
void tmr_Interrupt(int, Addr_t *reg)
{
    KernelPrintf("*");
    if (apic->tick && ThisCpu()->cpuNum == 0) apic->tick();
    apic->eoi();     // take care of this while interrupts are disabled!
//    uint64_t now = apic->currentTimer();
apic->currentTimer();

//    SchTimerTick(now);
}


