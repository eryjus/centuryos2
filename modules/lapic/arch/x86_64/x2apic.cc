//===================================================================================================================
//
//  x2apic.cc -- Functions to handle the x2apic initialization
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
// -- Convert the APIC MMIO offset into a MSR number (compiler should be able to optimize constants)
//    ----------------------------------------------------------------------------------------------
inline uint32_t GetX2apicMsr(ApicRegister_t reg)
{
    return (uint32_t)(reg >> 4) + 0x800;
}


//
// -- This is the spurious IRQ handler
//    --------------------------------
static void LApicSpurious(Addr_t *regs)
{
}


#if 0
//
// -- this is used during initialization to calibrate the timer
//    ---------------------------------------------------------
static void LApicInitTimeout(Addr_t *regs)
{
}
#endif



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
        case APIC_SELF_IPI:
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
    return X2APIC;
}


//
// -- Read an APIC register
//    ---------------------
static uint32_t ReadApicRegister(ApicRegister_t reg)
{
    if (!IsReadable(reg)) return 0;

    return RDMSR(GetX2apicMsr(reg));
}


//
// -- Write an APIC register
//    ----------------------
static void WriteApicRegister(ApicRegister_t reg, uint32_t val)
{
    if (!IsWritable(reg)) return;

    WRMSR(GetX2apicMsr(reg), val);
}


//
// -- Check APIC Status Register
//    --------------------------
static bool CheckApicStatus(ApicRegister_t reg, uint8_t index)
{
    if (!IsStatus(reg)) return false;

    int off = index / 32;
    int bit = index % 32;

    return (RDMSR(GetX2apicMsr(reg) + off) & (1 << bit)) != 0;
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

    KernelPrintf("Local x2APIC Init\n");


    if (isBoot) {
        // -- enable the APIC and x2apic mode
        WRMSR(IA32_APIC_BASE_MSR, 0
                | IA32_APIC_BASE_MSR__EN
                | IA32_APIC_BASE_MSR__EXTD
                | (apicBaseMsr & ~(PAGE_SIZE-1)));
        MmuDump(x2apic.baseAddr);
    }

    //
    // -- SW enable the Local APIC timer
    //    ------------------------------
    WriteApicRegister(APIC_ESR, 0);
    NOP();
    WriteApicRegister(APIC_SIVR, 39 | APIC_SOFTWARE_ENABLE);
    NOP();

    if (isBoot) {
        SetVectorHandler(INT_SPURIOUS, (Addr_t)LApicSpurious, GetAddressSpace(), 0);
    }

    // -- here we initialize the LAPIC to a defined state
    WriteApicRegister(APIC_DFR, 0xffffffff);
    WriteApicRegister(APIC_LVT_TIMER, APIC_LVT_MASKED);
    WriteApicRegister(APIC_LVT_PERF_COUNTING_REG, APIC_LVT_MASKED);
    WriteApicRegister(APIC_LVT_LINT0, APIC_LVT_MASKED);
    WriteApicRegister(APIC_LVT_LINT1, APIC_LVT_MASKED);
    WriteApicRegister(APIC_LVT_ERROR, APIC_LVT_MASKED);
    WriteApicRegister(APIC_LVT_THERMAL_SENSOR, 0);
    WriteApicRegister(APIC_TIMER_DCR, 0x03);      // divide value is 16
    WriteApicRegister(APIC_LVT_TIMER, 32);        // timer is vector 32; now unmasked


    // -- enable the PIC timer in one-shot mode
    if (isBoot) {
        KernelPrintf(".. Setting up the boot LAPIC\n");
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
        WriteApicRegister(APIC_TIMER_CCR, 0xffffffff);

        while (!(INB(0x61) & 0x20)) {}  // -- busy wait here

        WriteApicRegister(APIC_LVT_TIMER, APIC_LVT_MASKED);

        // -- remap the 8259 PIC to some obscure interrupts
        OUTB(0x20, 0x11);       // starts the initialization sequence (in cascade mode)
    	OUTB(0xa0, 0x11);
	    OUTB(0x21, 0x40);       // ICW2: Master PIC vector offset
	    OUTB(0xa1, 0x48);       // ICW2: Slave PIC vector offset
	    OUTB(0x21, 4);          // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	    OUTB(0xa1, 2);          // ICW3: tell Slave PIC its cascade identity (0000 0010)

        // -- disable the PIC
        OUTB(0x21, 0xff);
        OUTB(0xa1, 0xff);


        //
        // -- Now we can calculate the cpu frequency, converting back to a full second
        //    ------------------------------------------------------------------------
        uint64_t cpuFreq = (0xffffffff - ReadApicRegister(APIC_TIMER_CCR) * 16 * 20);
        x2apic.factor = cpuFreq / freq / 16;

        if (((((uint64_t)x2apic.factor) >> 32) & 0xffffffff) != 0) {
            KernelPrintf("PANIC: The factor is too large for the architecture!\n");
            while(true) {
                __asm volatile ("hlt");
            }
        }
    }

    //
    // -- Now, program the Timer
    //    ----------------------
    WriteApicRegister(APIC_TIMER_ICR, x2apic.factor);
    WriteApicRegister(APIC_LVT_TIMER, APIC_LVT_TIMER_PERIODIC | INT_TIMER);

    return 0;
}


//
// -- Called on each timer tick from CPU0
//    -----------------------------------
static void Tick(void)
{
    ticker += 1000;
}



//
// -- Get the current timer count
//    ---------------------------
static uint64_t CurrentTimer(void)
{
    return ticker;
}



//
// -- Get the LPAIC ID
//    ----------------
static int GetId(void)
{
    return (int)ReadApicRegister(APIC_LOCAL_ID);
}


//
// -- Send the INIT IPI
//    -----------------
static Return_t SendInit(int core)
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

    WriteApicRegister(APIC_ICR1, icr);

    return 0;
}


//
// -- Send the INIT IPI
//    -----------------
static Return_t SendSipi(int core, Addr_t vector)
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

    WriteApicRegister(APIC_ICR1, icr);

    return 0;
}


//
// -- Broadcast an IPI
//    ----------------
static Return_t SendIpi(int vector)
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

    WriteApicRegister(APIC_ICR1, icr);

//    KernelPrintf("ESR Result: %p\n", ReadApicRegister(APIC_ESR));

    return 0;
}


//
// -- Create the driver structure for the X2APIC
//    ------------------------------------------
Apic_t x2apic = {
    .baseAddr = 0,
    .factor = 0,
    .earlyInit = EarlyInit,
    .init = NULL,
    .getVersion = GetVersion,
    .readApicRegister = ReadApicRegister,
    .writeApicRegister = WriteApicRegister,
    .checkIndexedStatus = CheckApicStatus,
    .eoi = Eoi,
    .getId = GetId,
    .tick = Tick,
    .currentTimer = CurrentTimer,
    .sendInit = SendInit,
    .sendSipi = SendSipi,
    .sendIpi = SendIpi,
};


