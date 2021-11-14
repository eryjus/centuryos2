/****************************************************************************************************************//**
*   @file               lapic.h
*   @brief              Local APIC functions and structions
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



#pragma once


#include "types.h"
#include "boot-interface.h"



/****************************************************************************************************************//**
*   @enum               ApicVersion_t
*   @brief              Which kind of APIC are we talking about?
*///-----------------------------------------------------------------------------------------------------------------
enum ApicVersion_t {
    XAPIC = 1,                  //!< Original XAPIC
    X2APIC = 2,                 //!< X2APIC
};



/****************************************************************************************************************//**
*   @enum               ApicRegister_t
*   @brief              The registers that can be read/written in the Local APIC
*///-----------------------------------------------------------------------------------------------------------------
enum ApicRegister_t {
    APIC_LOCAL_ID = 0x20,               //!< Local APIC ID Register
    APIC_LOCAL_VERSION = 0x30,          //!< Local APIC Version Register
    APIC_TPR = 0x80,                    //!< Task Priority Register
    APIC_APR = 0x90,                    //!< Arbitration Priority Register
    APIC_PPR = 0xa0,                    //!< Processor Priority Register
    APIC_EOI = 0xb0,                    //!< EOI Register
    APIC_RRD = 0xc0,                    //!< Remote Read Register
    APIC_LDR = 0xd0,                    //!< Logical Destination Reister
    APIC_DFR = 0xe0,                    //!< Destination Format Register
    APIC_SIVR = 0xf0,                   //!< Spurious Interrupt Vector Register
    APIC_ISR_BASE = 0x100,              //!< In-Service Register Base
    APIC_TMR_BASE = 0x180,              //!< Trigger Mode Register Base
    APIC_IRR_BASE = 0x200,              //!< Interrupt Request Register Base
    APIC_ESR = 0x280,                   //!< Error Status Register
    APIC_CMCI = 0x2f0,                  //!< LVT Corrected Machine Check Interrupt Register
    APIC_ICR1 = 0x300,                  //!< Interrupt Command Register 1 (low bits)
    APIC_ICR2 = 0x310,                  //!< Interrupt Command REgister 2 (high bits)
    APIC_LVT_TIMER = 0x320,             //!< LVT Timer Register
    APIC_LVT_THERMAL_SENSOR = 0x330,    //!< LVT Thermal Sensor Register
    APIC_LVT_PERF_COUNTING_REG = 0x340, //!< LVT Performance Monitoring Counters Register
    APIC_LVT_LINT0 = 0x350,             //!< LVT LINT0 Register
    APIC_LVT_LINT1 = 0x360,             //!< LVT LINT1 Register
    APIC_LVT_ERROR = 0x370,             //!< LVT Error Register
    APIC_TIMER_ICR = 0x380,             //!< Inital Count Register (for Timer)
    APIC_TIMER_CCR = 0x390,             //!< Current Count Register (for Timer)
    APIC_TIMER_DCR = 0x3e0,             //!< Divide Configuration Register (for Timer; X2APIC only)
    APIC_SELF_IPI = 0x3f0,              //!< SELF IPI (X2APIC only)
};



/****************************************************************************************************************//**
*   @typedef            Apic_t
*   @brief              Formalization of the APIC Driver Structure into a type
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Apic_t
*   @brief              The APIC driver structure
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Apic_t {
    uint64_t baseAddr;                  //!< The base addess of the APIC Registers
    uint32_t factor;                    //!< The division factor to get the desired frequency
    uint64_t ticker;                    //!< The number of micro-seconds passed since boot
    ApicVersion_t version;              //!< The APIC version

    int (*earlyInit)(BootInterface_t *loaderInterface); //!< The early initialization function
    void (*init)(void);                 //!< The late initialization function

    uint32_t (*readApicRegister)(ApicRegister_t);   //!< Read an APIC register
    void (*writeApicRegister)(ApicRegister_t, uint32_t);    //!< Write to an APIC register
    void (*writeApicIcr)(uint64_t);     //!< Write to the APIC Interrupt Control Register

    bool (*checkIndexedStatus)(ApicRegister_t, uint8_t);    //!< Checked the status of an indexed register

    int (*getId)(void);                 //!< Get the current APIC ID
} Apic_t;



/****************************************************************************************************************//**
*   @var                apic
*   @brief              Pointer to the active type of APIC (XAPIC or X2APIC)
*///-----------------------------------------------------------------------------------------------------------------
extern Apic_t *apic;



/****************************************************************************************************************//**
*   @var                xapic
*   @brief              The XAPIC driver structure used to interact with the XAPIC
*///-----------------------------------------------------------------------------------------------------------------
extern Apic_t xapic;



/****************************************************************************************************************//**
*   @var                x2apic
*   @brief              The X2APIC driver structure used to interact with the X2APIC
*///-----------------------------------------------------------------------------------------------------------------
extern Apic_t x2apic;



/****************************************************************************************************************//**
*   @fn                 void LApicSpurious(Addr_t *regs)
*   @brief              Spurious function to be called
*
*   @param              regs                Pointer to the array of register values
*///-----------------------------------------------------------------------------------------------------------------
extern "C" void LApicSpurious(Addr_t *regs);



/****************************************************************************************************************//**
*   @fn                 bool IsReadable(ApicRegister_t reg)
*   @brief              Is the APIC register a readable register?
*
*   Reports if the specified APIC register is a readable one.
*
*   @param              reg                 The APIC register in question
*
*   @returns            Whether the APIC register is a readable register
*
*   @retval             true                The APIC register is a readable register
*   @retval             false               The APIC register is not a readable register
*///-----------------------------------------------------------------------------------------------------------------
extern "C" bool IsReadable(ApicRegister_t reg);



/****************************************************************************************************************//**
*   @fn                 bool IsWritable(ApicRegister_t reg)
*   @brief              Is the APIC register a writable register?
*
*   Reports if the specified APIC register is a writable one.
*
*   @param              reg                 The APIC register in question
*
*   @returns            Whether the APIC register is a writable register
*
*   @retval             true                The APIC register is a writable register
*   @retval             false               The APIC register is not a writable register
*///-----------------------------------------------------------------------------------------------------------------
extern "C" bool IsWritable(ApicRegister_t reg);



/****************************************************************************************************************//**
*   @fn                 bool IsStatus(ApicRegister_t reg)
*   @brief              Is the APIC register a status register?
*
*   Reports if the specified APIC register is a status one.
*
*   @param              reg                 The APIC register in question
*
*   @returns            Whether the APIC register is a status register
*
*   @retval             true                The APIC register is a status register
*   @retval             false               The APIC register is not a status register
*///-----------------------------------------------------------------------------------------------------------------
extern "C" bool IsStatus(ApicRegister_t reg);

