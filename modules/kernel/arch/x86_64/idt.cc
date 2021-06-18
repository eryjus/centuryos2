//===================================================================================================================
//
//  idt.cc -- handle interfacing with the IDT
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Jan-13  Initial  v0.0.2   ADCL  Initial version
//
//===================================================================================================================


#ifndef USE_SERIAL
#define USE_SERIAL
#endif

#include "types.h"
#include "printf.h"
#include "serial.h"
#include "internal.h"
#include "idt.h"


#if __has_include("tss.h")
#include "tss.h"

Tss_t tss[MAX_CPU] = { { 0 } };

#else

#define TssInit()

#endif




//
// -- Prototypes
//    ----------
extern IdtHandlerFunc_t IdtGenericEntry;
extern IdtHandlerFunc_t IdtGenericEntryNoErr;
extern IdtHandlerFunc_t isr32;
extern IdtHandlerFunc_t isr33;
extern IdtHandlerFunc_t isr34;
extern IdtHandlerFunc_t isr35;
extern IdtHandlerFunc_t isr36;
extern IdtHandlerFunc_t isr37;
extern IdtHandlerFunc_t isr38;
extern IdtHandlerFunc_t isr39;
extern IdtHandlerFunc_t isr40;
extern IdtHandlerFunc_t isr41;
extern IdtHandlerFunc_t isr42;
extern IdtHandlerFunc_t isr43;
extern IdtHandlerFunc_t isr44;
extern IdtHandlerFunc_t isr45;
extern IdtHandlerFunc_t isr46;
extern IdtHandlerFunc_t isr47;
extern IdtHandlerFunc_t isr48;
extern IdtHandlerFunc_t isr49;
extern IdtHandlerFunc_t isr50;
extern IdtHandlerFunc_t isr51;
extern IdtHandlerFunc_t isr52;
extern IdtHandlerFunc_t isr53;
extern IdtHandlerFunc_t isr54;
extern IdtHandlerFunc_t isr55;
extern IdtHandlerFunc_t isr56;
extern IdtHandlerFunc_t isr57;
extern IdtHandlerFunc_t isr58;
extern IdtHandlerFunc_t isr59;
extern IdtHandlerFunc_t isr60;
extern IdtHandlerFunc_t isr61;
extern IdtHandlerFunc_t isr62;
extern IdtHandlerFunc_t isr63;
extern IdtHandlerFunc_t isr64;
extern IdtHandlerFunc_t isr65;
extern IdtHandlerFunc_t isr66;
extern IdtHandlerFunc_t isr67;
extern IdtHandlerFunc_t isr68;
extern IdtHandlerFunc_t isr69;
extern IdtHandlerFunc_t isr70;
extern IdtHandlerFunc_t isr71;
extern IdtHandlerFunc_t isr72;
extern IdtHandlerFunc_t isr73;
extern IdtHandlerFunc_t isr74;
extern IdtHandlerFunc_t isr75;
extern IdtHandlerFunc_t isr76;
extern IdtHandlerFunc_t isr77;
extern IdtHandlerFunc_t isr78;
extern IdtHandlerFunc_t isr79;
extern IdtHandlerFunc_t isr80;
extern IdtHandlerFunc_t isr81;
extern IdtHandlerFunc_t isr82;
extern IdtHandlerFunc_t isr83;
extern IdtHandlerFunc_t isr84;
extern IdtHandlerFunc_t isr85;
extern IdtHandlerFunc_t isr86;
extern IdtHandlerFunc_t isr87;
extern IdtHandlerFunc_t isr88;
extern IdtHandlerFunc_t isr89;
extern IdtHandlerFunc_t isr90;
extern IdtHandlerFunc_t isr91;
extern IdtHandlerFunc_t isr92;
extern IdtHandlerFunc_t isr93;
extern IdtHandlerFunc_t isr94;
extern IdtHandlerFunc_t isr95;
extern IdtHandlerFunc_t isr96;
extern IdtHandlerFunc_t isr97;
extern IdtHandlerFunc_t isr98;
extern IdtHandlerFunc_t isr99;
extern IdtHandlerFunc_t isr100;
extern IdtHandlerFunc_t isr101;
extern IdtHandlerFunc_t isr102;
extern IdtHandlerFunc_t isr103;
extern IdtHandlerFunc_t isr104;
extern IdtHandlerFunc_t isr105;
extern IdtHandlerFunc_t isr106;
extern IdtHandlerFunc_t isr107;
extern IdtHandlerFunc_t isr108;
extern IdtHandlerFunc_t isr109;
extern IdtHandlerFunc_t isr110;
extern IdtHandlerFunc_t isr111;
extern IdtHandlerFunc_t isr112;
extern IdtHandlerFunc_t isr113;
extern IdtHandlerFunc_t isr114;
extern IdtHandlerFunc_t isr115;
extern IdtHandlerFunc_t isr116;
extern IdtHandlerFunc_t isr117;
extern IdtHandlerFunc_t isr118;
extern IdtHandlerFunc_t isr119;
extern IdtHandlerFunc_t isr120;
extern IdtHandlerFunc_t isr121;
extern IdtHandlerFunc_t isr122;
extern IdtHandlerFunc_t isr123;
extern IdtHandlerFunc_t isr124;
extern IdtHandlerFunc_t isr125;
extern IdtHandlerFunc_t isr126;
extern IdtHandlerFunc_t isr127;
extern IdtHandlerFunc_t isr128;
extern IdtHandlerFunc_t isr129;
extern IdtHandlerFunc_t isr130;
extern IdtHandlerFunc_t isr131;
extern IdtHandlerFunc_t isr132;
extern IdtHandlerFunc_t isr133;
extern IdtHandlerFunc_t isr134;
extern IdtHandlerFunc_t isr135;
extern IdtHandlerFunc_t isr136;
extern IdtHandlerFunc_t isr137;
extern IdtHandlerFunc_t isr138;
extern IdtHandlerFunc_t isr139;
extern IdtHandlerFunc_t isr140;
extern IdtHandlerFunc_t isr141;
extern IdtHandlerFunc_t isr142;
extern IdtHandlerFunc_t isr143;
extern IdtHandlerFunc_t isr144;
extern IdtHandlerFunc_t isr145;
extern IdtHandlerFunc_t isr146;
extern IdtHandlerFunc_t isr147;
extern IdtHandlerFunc_t isr148;
extern IdtHandlerFunc_t isr149;
extern IdtHandlerFunc_t isr150;
extern IdtHandlerFunc_t isr151;
extern IdtHandlerFunc_t isr152;
extern IdtHandlerFunc_t isr153;
extern IdtHandlerFunc_t isr154;
extern IdtHandlerFunc_t isr155;
extern IdtHandlerFunc_t isr156;
extern IdtHandlerFunc_t isr157;
extern IdtHandlerFunc_t isr158;
extern IdtHandlerFunc_t isr159;
extern IdtHandlerFunc_t isr160;
extern IdtHandlerFunc_t isr161;
extern IdtHandlerFunc_t isr162;
extern IdtHandlerFunc_t isr163;
extern IdtHandlerFunc_t isr164;
extern IdtHandlerFunc_t isr165;
extern IdtHandlerFunc_t isr166;
extern IdtHandlerFunc_t isr167;
extern IdtHandlerFunc_t isr168;
extern IdtHandlerFunc_t isr169;
extern IdtHandlerFunc_t isr170;
extern IdtHandlerFunc_t isr171;
extern IdtHandlerFunc_t isr172;
extern IdtHandlerFunc_t isr173;
extern IdtHandlerFunc_t isr174;
extern IdtHandlerFunc_t isr175;
extern IdtHandlerFunc_t isr176;
extern IdtHandlerFunc_t isr177;
extern IdtHandlerFunc_t isr178;
extern IdtHandlerFunc_t isr179;
extern IdtHandlerFunc_t isr180;
extern IdtHandlerFunc_t isr181;
extern IdtHandlerFunc_t isr182;
extern IdtHandlerFunc_t isr183;
extern IdtHandlerFunc_t isr184;
extern IdtHandlerFunc_t isr185;
extern IdtHandlerFunc_t isr186;
extern IdtHandlerFunc_t isr187;
extern IdtHandlerFunc_t isr188;
extern IdtHandlerFunc_t isr189;
extern IdtHandlerFunc_t isr190;
extern IdtHandlerFunc_t isr191;
extern IdtHandlerFunc_t isr192;
extern IdtHandlerFunc_t isr193;
extern IdtHandlerFunc_t isr194;
extern IdtHandlerFunc_t isr195;
extern IdtHandlerFunc_t isr196;
extern IdtHandlerFunc_t isr197;
extern IdtHandlerFunc_t isr198;
extern IdtHandlerFunc_t isr199;
extern IdtHandlerFunc_t isr200;
extern IdtHandlerFunc_t isr201;
extern IdtHandlerFunc_t isr202;
extern IdtHandlerFunc_t isr203;
extern IdtHandlerFunc_t isr204;
extern IdtHandlerFunc_t isr205;
extern IdtHandlerFunc_t isr206;
extern IdtHandlerFunc_t isr207;
extern IdtHandlerFunc_t isr208;
extern IdtHandlerFunc_t isr209;
extern IdtHandlerFunc_t isr210;
extern IdtHandlerFunc_t isr211;
extern IdtHandlerFunc_t isr212;
extern IdtHandlerFunc_t isr213;
extern IdtHandlerFunc_t isr214;
extern IdtHandlerFunc_t isr215;
extern IdtHandlerFunc_t isr216;
extern IdtHandlerFunc_t isr217;
extern IdtHandlerFunc_t isr218;
extern IdtHandlerFunc_t isr219;
extern IdtHandlerFunc_t isr220;
extern IdtHandlerFunc_t isr221;
extern IdtHandlerFunc_t isr222;
extern IdtHandlerFunc_t isr223;
extern IdtHandlerFunc_t isr224;
extern IdtHandlerFunc_t isr225;
extern IdtHandlerFunc_t isr226;
extern IdtHandlerFunc_t isr227;
extern IdtHandlerFunc_t isr228;
extern IdtHandlerFunc_t isr229;
extern IdtHandlerFunc_t isr230;
extern IdtHandlerFunc_t isr231;
extern IdtHandlerFunc_t isr232;
extern IdtHandlerFunc_t isr233;
extern IdtHandlerFunc_t isr234;
extern IdtHandlerFunc_t isr235;
extern IdtHandlerFunc_t isr236;
extern IdtHandlerFunc_t isr237;
extern IdtHandlerFunc_t isr238;
extern IdtHandlerFunc_t isr239;
extern IdtHandlerFunc_t isr240;
extern IdtHandlerFunc_t isr241;
extern IdtHandlerFunc_t isr242;
extern IdtHandlerFunc_t isr243;
extern IdtHandlerFunc_t isr244;
extern IdtHandlerFunc_t isr245;
extern IdtHandlerFunc_t isr246;
extern IdtHandlerFunc_t isr247;
extern IdtHandlerFunc_t isr248;
extern IdtHandlerFunc_t isr249;
extern IdtHandlerFunc_t isr250;
extern IdtHandlerFunc_t isr251;
extern IdtHandlerFunc_t isr252;
extern IdtHandlerFunc_t isr253;
extern IdtHandlerFunc_t isr254;
extern IdtHandlerFunc_t isr255;


extern "C" {
    void IdtGenericHandler(Addr_t *);
}



//
// -- This is the structure of an IDT Entry
//    -------------------------------------
typedef struct IdtDescriptor_t {
    unsigned int offsetLo : 16;
    unsigned int segmentSel : 16;
    unsigned int ist : 3;
    unsigned int zero : 5;
    unsigned int type : 4;
    unsigned int off : 1;
    unsigned int dpl : 2;
    unsigned int p : 1;
    unsigned int offsetMid : 16;
    unsigned int offsetHi : 32;
    unsigned int unused : 32;
} IdtDescriptor_t;


//
// -- The IDT Table
//    -------------
IdtDescriptor_t idtTable[256] __attribute__((aligned(4096))) = { {0} };


//
// -- This is the IDT Register data
//    -----------------------------
struct {
    uint16_t limit;
    Addr_t loc;
} __attribute__((packed)) idtr;



//
// -- This is the generic interrupt handler
//    -------------------------------------
void __attribute__((noreturn)) IdtGenericHandler(Addr_t *stack)
{
    enum {
        GS  = 1,
        FS  = 2,
        ES  = 3,
        DS  = 4,
        CR3 = 5,
        R15 = 6,
        R14 = 7,
        R13 = 8,
        R12 = 9,
        R11 = 10,
        R10 = 11,
        R9  = 12,
        R8  = 13,
        RDI = 14,
        RSI = 15,
        RBP = 16,
        RDX = 17,
        RCX = 18,
        RBX = 19,
        RAX = 20,
        INT = 21,
        ERR = 22,
        RIP = 23,
        CS  = 24,
        RFLAGS = 25,
        RSP = 26,
        SS  = 27,
    };

    kprintf("An unknown error (interrupt %x) has occurred (Error Code %p)\n", stack[INT], stack[ERR]);
    kprintf("  RAX: %p\tR8 : %p\n", stack[RAX], stack[R8]);
    kprintf("  RBX: %p\tR9 : %p\n", stack[RBX], stack[R9]);
    kprintf("  RCX: %p\tR10: %p\n", stack[RCX], stack[R10]);
    kprintf("  RDX: %p\tR11: %p\n", stack[RDX], stack[R11]);
    kprintf("  RBP: %p\tR12: %p\n", stack[RBP], stack[R12]);
    kprintf("  RSI: %p\tR13: %p\n", stack[RSI], stack[R13]);
    kprintf("  RDI: %p\tR14: %p\n", stack[RDI], stack[R14]);
    kprintf("\t\t\t\tR15: %p\n", stack[R15]);
    kprintf("   SS: %x\tRSP: %p\n", stack[SS], stack[RSP]);
    kprintf("   CS: %x\tRIP: %p\n", stack[CS], stack[RIP]);
    kprintf("   DS: %x\t ES: %x\n", stack[DS], stack[ES]);
    kprintf("   FS: %x\t GS: %x\n", stack[FS], stack[GS]);

    while (true) {}
}



extern "C" {
    void GsInit(void);
}


//
// -- Install the IDT
//    ---------------
#define ISR_TARGET(x) isr##x
#define ISR_VECTOR(x) case x: IdtSetHandler(x, 8, &ISR_TARGET(x), 0, 0); break;

void IdtInstall(void)
{
    for (int i = 0; i < 256; i ++) {
        switch (i) {
        case 8:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            IdtSetHandler(i, 8, &IdtGenericEntry, 0, 0);
            break;

        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 9:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
            IdtSetHandler(i, 8, &IdtGenericEntryNoErr, 0, 0);
            break;

        ISR_VECTOR(32)
        ISR_VECTOR(33)
        ISR_VECTOR(34)
        ISR_VECTOR(35)
        ISR_VECTOR(36)
        ISR_VECTOR(37)
        ISR_VECTOR(38)
        ISR_VECTOR(39)
        ISR_VECTOR(40)
        ISR_VECTOR(41)
        ISR_VECTOR(42)
        ISR_VECTOR(43)
        ISR_VECTOR(44)
        ISR_VECTOR(45)
        ISR_VECTOR(46)
        ISR_VECTOR(47)
        ISR_VECTOR(48)
        ISR_VECTOR(49)
        ISR_VECTOR(50)
        ISR_VECTOR(51)
        ISR_VECTOR(52)
        ISR_VECTOR(53)
        ISR_VECTOR(54)
        ISR_VECTOR(55)
        ISR_VECTOR(56)
        ISR_VECTOR(57)
        ISR_VECTOR(58)
        ISR_VECTOR(59)
        ISR_VECTOR(60)
        ISR_VECTOR(61)
        ISR_VECTOR(62)
        ISR_VECTOR(63)
        ISR_VECTOR(64)
        ISR_VECTOR(65)
        ISR_VECTOR(66)
        ISR_VECTOR(67)
        ISR_VECTOR(68)
        ISR_VECTOR(69)
        ISR_VECTOR(70)
        ISR_VECTOR(71)
        ISR_VECTOR(72)
        ISR_VECTOR(73)
        ISR_VECTOR(74)
        ISR_VECTOR(75)
        ISR_VECTOR(76)
        ISR_VECTOR(77)
        ISR_VECTOR(78)
        ISR_VECTOR(79)
        ISR_VECTOR(80)
        ISR_VECTOR(81)
        ISR_VECTOR(82)
        ISR_VECTOR(83)
        ISR_VECTOR(84)
        ISR_VECTOR(85)
        ISR_VECTOR(86)
        ISR_VECTOR(87)
        ISR_VECTOR(88)
        ISR_VECTOR(89)
        ISR_VECTOR(90)
        ISR_VECTOR(91)
        ISR_VECTOR(92)
        ISR_VECTOR(93)
        ISR_VECTOR(94)
        ISR_VECTOR(95)
        ISR_VECTOR(96)
        ISR_VECTOR(97)
        ISR_VECTOR(98)
        ISR_VECTOR(99)
        ISR_VECTOR(100)
        ISR_VECTOR(101)
        ISR_VECTOR(102)
        ISR_VECTOR(103)
        ISR_VECTOR(104)
        ISR_VECTOR(105)
        ISR_VECTOR(106)
        ISR_VECTOR(107)
        ISR_VECTOR(108)
        ISR_VECTOR(109)
        ISR_VECTOR(110)
        ISR_VECTOR(111)
        ISR_VECTOR(112)
        ISR_VECTOR(113)
        ISR_VECTOR(114)
        ISR_VECTOR(115)
        ISR_VECTOR(116)
        ISR_VECTOR(117)
        ISR_VECTOR(118)
        ISR_VECTOR(119)
        ISR_VECTOR(120)
        ISR_VECTOR(121)
        ISR_VECTOR(122)
        ISR_VECTOR(123)
        ISR_VECTOR(124)
        ISR_VECTOR(125)
        ISR_VECTOR(126)
        ISR_VECTOR(127)
        ISR_VECTOR(128)
        ISR_VECTOR(129)
        ISR_VECTOR(130)
        ISR_VECTOR(131)
        ISR_VECTOR(132)
        ISR_VECTOR(133)
        ISR_VECTOR(134)
        ISR_VECTOR(135)
        ISR_VECTOR(136)
        ISR_VECTOR(137)
        ISR_VECTOR(138)
        ISR_VECTOR(139)
        ISR_VECTOR(140)
        ISR_VECTOR(141)
        ISR_VECTOR(142)
        ISR_VECTOR(143)
        ISR_VECTOR(144)
        ISR_VECTOR(145)
        ISR_VECTOR(146)
        ISR_VECTOR(147)
        ISR_VECTOR(148)
        ISR_VECTOR(149)
        ISR_VECTOR(150)
        ISR_VECTOR(151)
        ISR_VECTOR(152)
        ISR_VECTOR(153)
        ISR_VECTOR(154)
        ISR_VECTOR(155)
        ISR_VECTOR(156)
        ISR_VECTOR(157)
        ISR_VECTOR(158)
        ISR_VECTOR(159)
        ISR_VECTOR(160)
        ISR_VECTOR(161)
        ISR_VECTOR(162)
        ISR_VECTOR(163)
        ISR_VECTOR(164)
        ISR_VECTOR(165)
        ISR_VECTOR(166)
        ISR_VECTOR(167)
        ISR_VECTOR(168)
        ISR_VECTOR(169)
        ISR_VECTOR(170)
        ISR_VECTOR(171)
        ISR_VECTOR(172)
        ISR_VECTOR(173)
        ISR_VECTOR(174)
        ISR_VECTOR(175)
        ISR_VECTOR(176)
        ISR_VECTOR(177)
        ISR_VECTOR(178)
        ISR_VECTOR(179)
        ISR_VECTOR(180)
        ISR_VECTOR(181)
        ISR_VECTOR(182)
        ISR_VECTOR(183)
        ISR_VECTOR(184)
        ISR_VECTOR(185)
        ISR_VECTOR(186)
        ISR_VECTOR(187)
        ISR_VECTOR(188)
        ISR_VECTOR(189)
        ISR_VECTOR(190)
        ISR_VECTOR(191)
        ISR_VECTOR(192)
        ISR_VECTOR(193)
        ISR_VECTOR(194)
        ISR_VECTOR(195)
        ISR_VECTOR(196)
        ISR_VECTOR(197)
        ISR_VECTOR(198)
        ISR_VECTOR(199)
        ISR_VECTOR(200)
        ISR_VECTOR(201)
        ISR_VECTOR(202)
        ISR_VECTOR(203)
        ISR_VECTOR(204)
        ISR_VECTOR(205)
        ISR_VECTOR(206)
        ISR_VECTOR(207)
        ISR_VECTOR(208)
        ISR_VECTOR(209)
        ISR_VECTOR(210)
        ISR_VECTOR(211)
        ISR_VECTOR(212)
        ISR_VECTOR(213)
        ISR_VECTOR(214)
        ISR_VECTOR(215)
        ISR_VECTOR(216)
        ISR_VECTOR(217)
        ISR_VECTOR(218)
        ISR_VECTOR(219)
        ISR_VECTOR(220)
        ISR_VECTOR(221)
        ISR_VECTOR(222)
        ISR_VECTOR(223)
        ISR_VECTOR(224)
        ISR_VECTOR(225)
        ISR_VECTOR(226)
        ISR_VECTOR(227)
        ISR_VECTOR(228)
        ISR_VECTOR(229)
        ISR_VECTOR(230)
        ISR_VECTOR(231)
        ISR_VECTOR(232)
        ISR_VECTOR(233)
        ISR_VECTOR(234)
        ISR_VECTOR(235)
        ISR_VECTOR(236)
        ISR_VECTOR(237)
        ISR_VECTOR(238)
        ISR_VECTOR(239)
        ISR_VECTOR(240)
        ISR_VECTOR(241)
        ISR_VECTOR(242)
        ISR_VECTOR(243)
        ISR_VECTOR(244)
        ISR_VECTOR(245)
        ISR_VECTOR(246)
        ISR_VECTOR(247)
        ISR_VECTOR(248)
        ISR_VECTOR(249)
        ISR_VECTOR(250)
        ISR_VECTOR(251)
        ISR_VECTOR(252)
        ISR_VECTOR(253)
        ISR_VECTOR(254)
        ISR_VECTOR(255)
        }
    }

    idtr.loc = (Addr_t)idtTable;
    idtr.limit = sizeof(idtTable) - 1;

    __asm__("lidt %0" : : "m"(idtr));

    IdtSetHandler(0xe0, 8, &InternalTarget, 0, 0);

    TssInit();
    GsInit();
}
#undef ISR_VECTOR
#undef ISR_TARGET



//
// -- Get the address of an existing IDT Handler
//    ------------------------------------------
Addr_t IdtGetHandler(int i)
{
    IdtDescriptor_t desc = idtTable[i];
    Addr_t rv = 0;

    rv |= ((Addr_t)desc.offsetHi << 32);
    rv |= ((Addr_t)desc.offsetMid << 16);
    rv |= ((Addr_t)desc.offsetLo);

    return rv;
}



//
// - Set the address of a new IDT Handler
//   ------------------------------------
void IdtSetHandler(int i, uint16_t sec, IdtHandlerFunc_t *handler, int ist, int dpl)
{
    if (!handler) {
        IdtSetHandler(i, sec, &IdtGenericEntry, ist, dpl);
        return;
    }

    IdtDescriptor_t desc;

    desc.offsetLo = ((Addr_t)handler) & 0xffff;
    desc.segmentSel = sec;
    desc.ist = ist;
    desc.zero = 0;
    desc.type = 0xe;
    desc.off = 0;
    desc.dpl = dpl;
    desc.p = 1;
    desc.offsetMid = (((Addr_t)handler) >> 16) & 0xffff;
    desc.offsetHi = (((Addr_t)handler) >> 32) & 0xffffffff;
    desc.unused = 0;

    idtTable[i] = desc;
}



//
// -- Set an interrupt vector handler
//    -------------------------------
int krn_SetVectorHandler(int i, IdtHandlerFunc_t handler, Addr_t cr3)
{
    if (i < 0 || i >= 256) return -EINVAL;
    vectorTable[i].handler = handler;
    vectorTable[i].cr3 = cr3;
    return 0;
}




