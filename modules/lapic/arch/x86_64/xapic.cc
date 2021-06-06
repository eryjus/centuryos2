//===================================================================================================================
//
//  xapic.cc -- Functions to handle the xapic initialization
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-May-06  Initial  v0.0.8   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "boot-interface.h"
#include "kernel-funcs.h"
#include "lapic.h"



//
// -- The current tick count
//    ----------------------
static uint64_t ticker = 0;



//
// -- This is the spurious IRQ handler
//    --------------------------------
static void LApicSpurious(Addr_t *regs)
{
}


//
// -- this is used during initialization to calibrate the timer
//    ---------------------------------------------------------
static void LApicInitTimeout(Addr_t *regs)
{
}



//
// -- check if the register is readable
//    ---------------------------------
static bool IsReadable(ApicRegister_t reg)
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
            // APIC_ICR2 is not availble is x2apic mode
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

        default:
            return false;
    }
}


//
// -- check if the register is writable
//    ----------------------------------
static bool IsWritable(ApicRegister_t reg)
{
    switch (reg) {
        case APIC_TPR:
        case APIC_EOI:
        case APIC_SIVR:
        case APIC_ESR:
        case APIC_CMCI:
        case APIC_ICR1:
            // APIC_ICR2 is not availble is x2apic mode
        case APIC_LVT_TIMER:
        case APIC_LVT_THERMAL_SENSOR:
        case APIC_LVT_PERF_COUNTING_REG:
        case APIC_LVT_LINT0:
        case APIC_LVT_LINT1:
        case APIC_LVT_ERROR:
        case APIC_TIMER_ICR:
        case APIC_TIMER_DCR:
            return true;

        default:
            return false;
    }
}

//
// -- Check for a status register
//    ---------------------------
static bool IsStatus(ApicRegister_t reg)
{
    if (reg == APIC_ISR_BASE || reg == APIC_TMR_BASE || reg == APIC_IRR_BASE) return true;
    else return false;
}



//
// -- Get the APIC hardware version (xAPIC or x2APIC)
//    -----------------------------------------------
static ApicVersion_t GetVersion(void)
{
    return XAPIC;
}


//
// -- Read an APIC register
//    ---------------------
static uint32_t ReadApicRegister(ApicRegister_t reg)
{
    if (!IsReadable(reg)) return 0;

    return PEEK32(xapic.baseAddr + reg);
}


//
// -- Write an APIC register
//    ----------------------
static void WriteApicRegister(ApicRegister_t reg, uint32_t val)
{
    if (!IsWritable(reg)) return;

    POKE32(xapic.baseAddr + reg, val);
}


//
// -- Check APIC Status Register
//    --------------------------
static bool CheckApicStatus(ApicRegister_t reg, uint8_t index)
{
    if (!IsStatus(reg)) return false;

    int off = index / 32;
    int bit = index % 32;

    return (PEEK32(xapic.baseAddr + reg + off) & (1 << bit)) != 0;
}


//
// -- Issue an EOI
//    ------------
static void Eoi(void)
{
    WriteApicRegister(APIC_EOI, 0);
}


//
// -- Early Initialization function
//    NOTE: when this function is called with `loaderInterface != 0`, then it is the original
//    initialization.  When `loaderInterface == NULL`, then it is subsequent initialization for
//    additional boot processors.  Additionally, IA32_APIC_BASE_MSR__BSP also indicates that this
//    the BSP.
//    -------------------------------------------------------------------------------------------
static int EarlyInit(BootInterface_t *loaderInterface)
{
    static int freq = 1000;
    uint64_t apicBaseMsr = RDMSR(IA32_APIC_BASE_MSR);
    bool isBoot = (apicBaseMsr & IA32_APIC_BASE_MSR__BSP) != 0;
    Frame_t apicFrame = apicBaseMsr >> 12;

    KernelPrintf("Local xAPIC Init\n");

    if (isBoot) {
        // -- enable the APIC and x2apic mode
        WRMSR(IA32_APIC_BASE_MSR, 0
                | IA32_APIC_BASE_MSR__EN
                | (apicBaseMsr & 0xfff));

        MmuMapPage(xapic.baseAddr, apicFrame, true);
    }

    //
    // -- SW enable the Local APIC timer
    //    ------------------------------
    WriteApicRegister(APIC_ESR, 0);
    __asm volatile("nop\n");
    WriteApicRegister(APIC_SIVR, 39 | IA32_APIC_BASE_MSR__EN);
    __asm volatile("nop\n");


    if (isBoot) {
        SetInterruptHandler(32, 0x08, (Addr_t)LApicInitTimeout, 0, 0);
        SetInterruptHandler(32, 0x08, (Addr_t)LApicSpurious, 0, 0);
    }


    KernelPrintf(".. Initializing to a defined state\n");

    // -- here we initialize the LAPIC to a defined state -- taken from Century32
    WriteApicRegister(APIC_DFR, 0xffffffff);       // ipi flat model??
    WriteApicRegister(APIC_LDR, ReadApicRegister(APIC_LDR) | (1<<24));    // set logical apic to 1
    WriteApicRegister(APIC_LVT_TIMER, APIC_LVT_MASKED);           // mask the timer during setup
    WriteApicRegister(APIC_LVT_PERF_COUNTING_REG, APIC_LVT_MASKED);
    WriteApicRegister(APIC_LVT_LINT0, APIC_LVT_MASKED);
    WriteApicRegister(APIC_LVT_LINT1, APIC_LVT_MASKED);
    WriteApicRegister(APIC_LVT_ERROR, APIC_LVT_MASKED);
    WriteApicRegister(APIC_TPR, 0);
    WriteApicRegister(APIC_TIMER_DCR, 0x03);       // divide value is 16
    WriteApicRegister(APIC_LVT_TIMER, 32);        // timer is vector 32; now unmasked


    // -- enable the PIC timer in one-shot mode
    if (isBoot) {
        OUTB(0x61, (INB(0x61) & 0xfd) | 1);
        OUTB(0x43, 0xb2);

        //
        // -- So, here is the math:
        //    We need to divide the clock by 20 to have a value large enough to get a decent time.
        //    So, we will be measuring 1/20th of a second.
        // -- 1193180 Hz / 20 == 59659 cycles == e90b cycles
        OUTB(0x42, 0x0b);
        INB(0x60);      // short delay
        OUTB(0x42, 0xe9);

        // -- now reset the PIT timer and start counting
        uint8_t tmp = INB(0x61) & 0xfe;
        OUTB(0x61, tmp);
        OUTB(0x61, tmp | 1);

        // -- reset the APIC counter to -1
        WriteApicRegister(APIC_TIMER_ICR, 0xffffffff);

        while (!(INB(0x61) & 0x20)) {}  // -- busy wait here

        WriteApicRegister(APIC_LVT_TIMER, APIC_LVT_MASKED);

        // -- disable the PIC
        OUTB(0x21, 0xff);
        OUTB(0xa1, 0xff);


        //
        // -- Now we can calculate the cpu frequency, converting back to a full second
        //    ------------------------------------------------------------------------
        uint64_t cpuFreq = (0xffffffff - ReadApicRegister(APIC_TIMER_CCR)) * 16 * 20;
        xapic.factor = cpuFreq / freq / 16;

        if (((((uint64_t)x2apic.factor) >> 32) & 0xffffffff) != 0) {
            KernelPrintf("PANIC: The factor is too large for the architecture!\n");
            while(true) {
                __asm volatile ("hlt");
            }
        }
    }

    KernelPrintf(".. Finally program the timer\n");

    //
    // -- Now, program the Timer
    //    ----------------------
    WriteApicRegister(APIC_TIMER_ICR, xapic.factor);
    WriteApicRegister(APIC_LVT_TIMER, APIC_LVT_TIMER_PERIODIC | 32);

    return 0;
}


//
// -- Called on each timer tick from CPU0
//    -----------------------------------
static void Tick(void)
{
    ticker += 1000000;
}


//
// -- Get the current timer count
//    ---------------------------
static uint64_t CurrentTimer(void)
{
    return ticker;
}


//
// -- Create the driver structure for the X2APIC
//    ------------------------------------------
Apic_t xapic = {
    .baseAddr = LAPIC_MMIO,
    .factor = 0,
    .earlyInit = EarlyInit,
    .init = NULL,
    .getVersion = GetVersion,
    .readApicRegister = ReadApicRegister,
    .writeApicRegister = WriteApicRegister,
    .checkIndexedStatus = CheckApicStatus,
    .eoi = Eoi,
    .tick = Tick,
    .currentTimer = CurrentTimer,
};


