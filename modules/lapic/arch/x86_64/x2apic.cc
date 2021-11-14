/****************************************************************************************************************//**
*   @file               x2apic.cc
*   @brief              Functions to handle the X2APIC
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-May-06
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
*   | 2021-May-06 | Initial |  v0.0.08 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "boot-interface.h"
#include "kernel-funcs.h"
#include "lapic.h"



/********************************************************************************************************************
*   See documentation in `lapic.h`
*///-----------------------------------------------------------------------------------------------------------------
extern Apic_t x2apic;



/****************************************************************************************************************//**
*   @fn                 uint32_t GetX2apicMsr(ApicRegister_t reg)
*   @brief              Convert the APIC MMIO offset into a MSR number
*
*   Convert the APIC MMIO offset into a MSR number
*
*   @param              reg                 The APIC register in question
*
*   @returns            The contents of the register read; 0 if an invalid register
*///-----------------------------------------------------------------------------------------------------------------
inline uint32_t GetX2apicMsr(ApicRegister_t reg)
{
    return (uint32_t)(reg >> 4) + 0x800;
}




/****************************************************************************************************************//**
*   @fn                 uint32_t ReadX2apicRegister(ApicRegister_t reg)
*   @brief              Read an APIC register
*
*   Read an XAPIC register and return its contents
*
*   @param              reg                 The APIC register in question
*
*   @returns            The contents of the register read; 0 if an invalid register
*///-----------------------------------------------------------------------------------------------------------------
static uint32_t ReadX2apicRegister(ApicRegister_t reg)
{
    if (!IsReadable(reg)) return 0;

    return RDMSR(GetX2apicMsr(reg));
}


/****************************************************************************************************************//**
*   @fn                 void WriteX2apicRegister(ApicRegister_t reg, uint32_t val)
*   @brief              Write an APIC register
*
*   Write a value to an XAPIC register
*
*   @param              reg                 The APIC register in question
*   @param              val                 The value to write to the register
*///-----------------------------------------------------------------------------------------------------------------
static void WriteX2apicRegister(ApicRegister_t reg, uint32_t val)
{
    if (!IsWritable(reg)) return;

    WRMSR(GetX2apicMsr(reg), val);
}


/****************************************************************************************************************//**
*   @fn                 bool CheckX2apicStatus(ApicRegister_t reg, uint8_t index)
*   @brief              Check APIC Status Register
*
*   Check an XAPIC status register and return if it is set
*
*   @param              reg                 The APIC register in question
*   @param              index               The index of the register status to check
*
*   @returns            If the status bit is set
*
*   @retval             false               The status bit is not set
*   @retval             true                The status but is set
*///-----------------------------------------------------------------------------------------------------------------
static bool CheckX2apicStatus(ApicRegister_t reg, uint8_t index)
{
    if (!IsStatus(reg)) return false;

    int off = index / 32;
    int bit = index % 32;

    return (RDMSR(GetX2apicMsr(reg) + off) & (1 << bit)) != 0;
}



/****************************************************************************************************************//**
*   @fn                 int EarlyInit(BootInterface_t *loaderInterface)
*   @brief              Early Initialization function
*
*   @note               when this function is called with `loaderInterface != 0`, then it is the original
*                       initialization.  When `loaderInterface == NULL`, then it is subsequent initialization for
*                       additional boot processors.  Additionally, IA32_APIC_BASE_MSR__BSP also indicates that this
*                       the BSP.
*
*   @param              loaderInterface     The Loader Interface structure containing hardware info
*
*   @returns            0
*///-----------------------------------------------------------------------------------------------------------------
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
    WriteX2apicRegister(APIC_ESR, 0);
    NOP();
    WriteX2apicRegister(APIC_SIVR, 39 | APIC_SOFTWARE_ENABLE);
    NOP();

    if (isBoot) {
        SetVectorHandler(INT_SPURIOUS, (Addr_t)LApicSpurious, GetAddressSpace(), 0);
    }

    // -- here we initialize the LAPIC to a defined state
    WriteX2apicRegister(APIC_DFR, 0xffffffff);
    WriteX2apicRegister(APIC_LVT_TIMER, APIC_LVT_MASKED);
    WriteX2apicRegister(APIC_LVT_PERF_COUNTING_REG, APIC_LVT_MASKED);
    WriteX2apicRegister(APIC_LVT_LINT0, APIC_LVT_MASKED);
    WriteX2apicRegister(APIC_LVT_LINT1, APIC_LVT_MASKED);
    WriteX2apicRegister(APIC_LVT_ERROR, APIC_LVT_MASKED);
    WriteX2apicRegister(APIC_LVT_THERMAL_SENSOR, 0);
    WriteX2apicRegister(APIC_TIMER_DCR, 0x03);      // divide value is 16
    WriteX2apicRegister(APIC_LVT_TIMER, 32);        // timer is vector 32; now unmasked


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
        WriteX2apicRegister(APIC_TIMER_CCR, 0xffffffff);

        while (!(INB(0x61) & 0x20)) {}  // -- busy wait here

        WriteX2apicRegister(APIC_LVT_TIMER, APIC_LVT_MASKED);

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
        uint64_t cpuFreq = (0xffffffff - ReadX2apicRegister(APIC_TIMER_CCR) * 16 * 20);
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
    WriteX2apicRegister(APIC_TIMER_ICR, x2apic.factor);
    WriteX2apicRegister(APIC_LVT_TIMER, APIC_LVT_TIMER_PERIODIC | INT_TIMER);

    return 0;
}



/****************************************************************************************************************//**
*   @fn                 int GetId(void)
*   @brief              Get the LPAIC ID
*
*   Get the Local APIC ID, which will be the CPU number
*
*   @returns            CPU number
*///-----------------------------------------------------------------------------------------------------------------
static int GetId(void)
{
    return (int)ReadX2apicRegister(APIC_LOCAL_ID);
}



/****************************************************************************************************************//**
*   @fn                 void WriteX2apicIcr(uint64_t val)
*   @brief              Write 64-bits to the ICR register
*
*   Write 64-bits to the ICR register
*
*   @param              val                 The value to write to the ICR register
*///-----------------------------------------------------------------------------------------------------------------
static void WriteX2apicIcr(uint64_t val)
{
    WriteX2apicRegister(APIC_ICR2, val);
}



/********************************************************************************************************************
*   See documentation in `lapic.h`
*///-----------------------------------------------------------------------------------------------------------------
Apic_t x2apic = {
    .baseAddr = 0,
    .factor = 0,
    .ticker = 0,
    .version = X2APIC,
    .earlyInit = EarlyInit,
    .init = NULL,
    .readApicRegister = ReadX2apicRegister,
    .writeApicRegister = WriteX2apicRegister,
    .writeApicIcr = WriteX2apicIcr,
    .checkIndexedStatus = CheckX2apicStatus,
    .getId = GetId,
};


