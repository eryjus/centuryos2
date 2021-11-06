//===================================================================================================================
//
//  tss.h -- architecture-specific structures for the x86-64 TSS
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-27  Initial  v0.0.4   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"


#pragma once
#ifndef __TSS_H__
#define __TSS_H__


//
// -- The 64-bit TSS
//    --------------
typedef struct Tss_t {
    uint32_t reserved0;
    uint32_t lowerRsp0;
    uint32_t upperRsp0;
    uint32_t lowerRsp1;
    uint32_t upperRsp1;
    uint32_t lowerRsp2;
    uint32_t upperRsp2;
    uint32_t lowerRsp3;
    uint32_t upperRsp3;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t lowerIst1;
    uint32_t upperIst1;
    uint32_t lowerIst2;
    uint32_t upperIst2;
    uint32_t lowerIst3;
    uint32_t upperIst3;
    uint32_t lowerIst4;
    uint32_t upperIst4;
    uint32_t lowerIst5;
    uint32_t upperIst5;
    uint32_t lowerIst6;
    uint32_t upperIst6;
    uint32_t lowerIst7;
    uint32_t upperIst7;
    uint32_t reserved3;
    uint32_t reserved4;
    uint16_t reserved5;
    uint16_t ioMapBase;
} __attribute__((packed)) Tss_t;


//
// -- some access functions to any given TSS
//    --------------------------------------
inline uint64_t GetRps0(Tss_t *tss) { return (((uint64_t)tss->upperRsp0) << 32) | tss->lowerRsp0; }
inline void SetRsp0(Tss_t *tss, uint64_t addr) {
    tss->upperRsp0 = (uint32_t)(addr >> 32);
    tss->lowerRsp0 = (uint32_t)(addr & 0xffffffff);
}
inline uint64_t GetRps1(Tss_t *tss) { return (((uint64_t)tss->upperRsp1) << 32) | tss->lowerRsp1; }
inline void SetRsp1(Tss_t *tss, uint64_t addr) {
    tss->upperRsp1 = (uint32_t)(addr >> 32);
    tss->lowerRsp1 = (uint32_t)(addr & 0xffffffff);
}
inline uint64_t GetRps2(Tss_t *tss) { return (((uint64_t)tss->upperRsp2) << 32) | tss->lowerRsp2; }
inline void SetRsp2(Tss_t *tss, uint64_t addr) {
    tss->upperRsp2 = (uint32_t)(addr >> 32);
    tss->lowerRsp2 = (uint32_t)(addr & 0xffffffff);
}
inline uint64_t GetRps3(Tss_t *tss) { return (((uint64_t)tss->upperRsp3) << 32) | tss->lowerRsp3; }
inline void SetRsp3(Tss_t *tss, uint64_t addr) {
    tss->upperRsp3 = (uint32_t)(addr >> 32);
    tss->lowerRsp3 = (uint32_t)(addr & 0xffffffff);
}

inline uint64_t GetIst1(Tss_t *tss) { return (((uint64_t)tss->upperIst1) << 32) | tss->lowerIst1; }
inline void SetIst1(Tss_t *tss, uint64_t addr) {
    tss->upperIst1 = (uint32_t)(addr >> 32);
    tss->lowerIst1 = (uint32_t)(addr & 0xffffffff);
}
inline uint64_t GetIst2(Tss_t *tss) { return (((uint64_t)tss->upperIst2) << 32) | tss->lowerIst2; }
inline void SetIst2(Tss_t *tss, uint64_t addr) {
    tss->upperIst2 = (uint32_t)(addr >> 32);
    tss->lowerIst2 = (uint32_t)(addr & 0xffffffff);
}
inline uint64_t GetIst3(Tss_t *tss) { return (((uint64_t)tss->upperIst3) << 32) | tss->lowerIst3; }
inline void SetIst3(Tss_t *tss, uint64_t addr) {
    tss->upperIst3 = (uint32_t)(addr >> 32);
    tss->lowerIst3 = (uint32_t)(addr & 0xffffffff);
}
inline uint64_t GetIst4(Tss_t *tss) { return (((uint64_t)tss->upperIst4) << 32) | tss->lowerIst4; }
inline void SetIst4(Tss_t *tss, uint64_t addr) {
    tss->upperIst4 = (uint32_t)(addr >> 32);
    tss->lowerIst4 = (uint32_t)(addr & 0xffffffff);
}
inline uint64_t GetIst5(Tss_t *tss) { return (((uint64_t)tss->upperIst5) << 32) | tss->lowerIst5; }
inline void SetIst5(Tss_t *tss, uint64_t addr) {
    tss->upperIst5 = (uint32_t)(addr >> 32);
    tss->lowerIst5 = (uint32_t)(addr & 0xffffffff);
}
inline uint64_t GetIst6(Tss_t *tss) { return (((uint64_t)tss->upperIst6) << 32) | tss->lowerIst6; }
inline void SetIst6(Tss_t *tss, uint64_t addr) {
    tss->upperIst6 = (uint32_t)(addr >> 32);
    tss->lowerIst6 = (uint32_t)(addr & 0xffffffff);
}
inline uint64_t GetIst7(Tss_t *tss) { return (((uint64_t)tss->upperIst7) << 32) | tss->lowerIst7; }
inline void SetIst7(Tss_t *tss, uint64_t addr) {
    tss->upperIst7 = (uint32_t)(addr >> 32);
    tss->lowerIst7 = (uint32_t)(addr & 0xffffffff);
}

inline uint16_t GetIoBase(Tss_t *tss) { return tss->ioMapBase; }
inline void SetIoBase(Tss_t *tss, uint16_t base) { tss->ioMapBase = base; }


//
// -- Some external prototypes
//    ------------------------
extern "C" {
    void TssInit(void);
    Addr_t LoadCr3(Addr_t n);
}


//
// -- Expose the TSS for the static GDT addresses
//    -------------------------------------------
extern Tss_t tss[MAX_CPU];

#endif

