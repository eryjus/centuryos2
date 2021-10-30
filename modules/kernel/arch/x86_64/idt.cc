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



#include "types.h"
#include "printf.h"
#include "serial.h"
#include "internals.h"
#include "scheduler.h"
#include "kernel-funcs.h"
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
extern Addr_t int0;
extern Addr_t int1;
extern Addr_t int2;
extern Addr_t int3;
extern Addr_t int4;
extern Addr_t int5;
extern Addr_t int6;
extern Addr_t int7;
extern Addr_t int8;
extern Addr_t int9;
extern Addr_t int10;
extern Addr_t int11;
extern Addr_t int12;
extern Addr_t int13;
extern Addr_t int14;
extern Addr_t int15;
extern Addr_t int16;
extern Addr_t int17;
extern Addr_t int18;
extern Addr_t int19;
extern Addr_t int20;
extern Addr_t int21;
extern Addr_t int22;
extern Addr_t int23;
extern Addr_t int24;
extern Addr_t int25;
extern Addr_t int26;
extern Addr_t int27;
extern Addr_t int28;
extern Addr_t int29;
extern Addr_t int30;
extern Addr_t int31;

extern Addr_t isr32;
extern Addr_t isr33;
extern Addr_t isr34;
extern Addr_t isr35;
extern Addr_t isr36;
extern Addr_t isr37;
extern Addr_t isr38;
extern Addr_t isr39;
extern Addr_t isr40;
extern Addr_t isr41;
extern Addr_t isr42;
extern Addr_t isr43;
extern Addr_t isr44;
extern Addr_t isr45;
extern Addr_t isr46;
extern Addr_t isr47;
extern Addr_t isr48;
extern Addr_t isr49;
extern Addr_t isr50;
extern Addr_t isr51;
extern Addr_t isr52;
extern Addr_t isr53;
extern Addr_t isr54;
extern Addr_t isr55;
extern Addr_t isr56;
extern Addr_t isr57;
extern Addr_t isr58;
extern Addr_t isr59;
extern Addr_t isr60;
extern Addr_t isr61;
extern Addr_t isr62;
extern Addr_t isr63;
extern Addr_t isr64;
extern Addr_t isr65;
extern Addr_t isr66;
extern Addr_t isr67;
extern Addr_t isr68;
extern Addr_t isr69;
extern Addr_t isr70;
extern Addr_t isr71;
extern Addr_t isr72;
extern Addr_t isr73;
extern Addr_t isr74;
extern Addr_t isr75;
extern Addr_t isr76;
extern Addr_t isr77;
extern Addr_t isr78;
extern Addr_t isr79;
extern Addr_t isr80;
extern Addr_t isr81;
extern Addr_t isr82;
extern Addr_t isr83;
extern Addr_t isr84;
extern Addr_t isr85;
extern Addr_t isr86;
extern Addr_t isr87;
extern Addr_t isr88;
extern Addr_t isr89;
extern Addr_t isr90;
extern Addr_t isr91;
extern Addr_t isr92;
extern Addr_t isr93;
extern Addr_t isr94;
extern Addr_t isr95;
extern Addr_t isr96;
extern Addr_t isr97;
extern Addr_t isr98;
extern Addr_t isr99;
extern Addr_t isr100;
extern Addr_t isr101;
extern Addr_t isr102;
extern Addr_t isr103;
extern Addr_t isr104;
extern Addr_t isr105;
extern Addr_t isr106;
extern Addr_t isr107;
extern Addr_t isr108;
extern Addr_t isr109;
extern Addr_t isr110;
extern Addr_t isr111;
extern Addr_t isr112;
extern Addr_t isr113;
extern Addr_t isr114;
extern Addr_t isr115;
extern Addr_t isr116;
extern Addr_t isr117;
extern Addr_t isr118;
extern Addr_t isr119;
extern Addr_t isr120;
extern Addr_t isr121;
extern Addr_t isr122;
extern Addr_t isr123;
extern Addr_t isr124;
extern Addr_t isr125;
extern Addr_t isr126;
extern Addr_t isr127;
extern Addr_t isr128;
extern Addr_t isr129;
extern Addr_t isr130;
extern Addr_t isr131;
extern Addr_t isr132;
extern Addr_t isr133;
extern Addr_t isr134;
extern Addr_t isr135;
extern Addr_t isr136;
extern Addr_t isr137;
extern Addr_t isr138;
extern Addr_t isr139;
extern Addr_t isr140;
extern Addr_t isr141;
extern Addr_t isr142;
extern Addr_t isr143;
extern Addr_t isr144;
extern Addr_t isr145;
extern Addr_t isr146;
extern Addr_t isr147;
extern Addr_t isr148;
extern Addr_t isr149;
extern Addr_t isr150;
extern Addr_t isr151;
extern Addr_t isr152;
extern Addr_t isr153;
extern Addr_t isr154;
extern Addr_t isr155;
extern Addr_t isr156;
extern Addr_t isr157;
extern Addr_t isr158;
extern Addr_t isr159;
extern Addr_t isr160;
extern Addr_t isr161;
extern Addr_t isr162;
extern Addr_t isr163;
extern Addr_t isr164;
extern Addr_t isr165;
extern Addr_t isr166;
extern Addr_t isr167;
extern Addr_t isr168;
extern Addr_t isr169;
extern Addr_t isr170;
extern Addr_t isr171;
extern Addr_t isr172;
extern Addr_t isr173;
extern Addr_t isr174;
extern Addr_t isr175;
extern Addr_t isr176;
extern Addr_t isr177;
extern Addr_t isr178;
extern Addr_t isr179;
extern Addr_t isr180;
extern Addr_t isr181;
extern Addr_t isr182;
extern Addr_t isr183;
extern Addr_t isr184;
extern Addr_t isr185;
extern Addr_t isr186;
extern Addr_t isr187;
extern Addr_t isr188;
extern Addr_t isr189;
extern Addr_t isr190;
extern Addr_t isr191;
extern Addr_t isr192;
extern Addr_t isr193;
extern Addr_t isr194;
extern Addr_t isr195;
extern Addr_t isr196;
extern Addr_t isr197;
extern Addr_t isr198;
extern Addr_t isr199;
extern Addr_t isr200;
extern Addr_t isr201;
extern Addr_t isr202;
extern Addr_t isr203;
extern Addr_t isr204;
extern Addr_t isr205;
extern Addr_t isr206;
extern Addr_t isr207;
extern Addr_t isr208;
extern Addr_t isr209;
extern Addr_t isr210;
extern Addr_t isr211;
extern Addr_t isr212;
extern Addr_t isr213;
extern Addr_t isr214;
extern Addr_t isr215;
extern Addr_t isr216;
extern Addr_t isr217;
extern Addr_t isr218;
extern Addr_t isr219;
extern Addr_t isr220;
extern Addr_t isr221;
extern Addr_t isr222;
extern Addr_t isr223;
extern Addr_t isr224;
extern Addr_t isr225;
extern Addr_t isr226;
extern Addr_t isr227;
extern Addr_t isr228;
extern Addr_t isr229;
extern Addr_t isr230;
extern Addr_t isr231;
extern Addr_t isr232;
extern Addr_t isr233;
extern Addr_t isr234;
extern Addr_t isr235;
extern Addr_t isr236;
extern Addr_t isr237;
extern Addr_t isr238;
extern Addr_t isr239;
extern Addr_t isr240;
extern Addr_t isr241;
extern Addr_t isr242;
extern Addr_t isr243;
extern Addr_t isr244;
extern Addr_t isr245;
extern Addr_t isr246;
extern Addr_t isr247;
extern Addr_t isr248;
extern Addr_t isr249;
extern Addr_t isr250;
extern Addr_t isr251;
extern Addr_t isr252;
extern Addr_t isr253;
extern Addr_t isr254;
extern Addr_t isr255;

extern Addr_t InternalTarget;
extern Addr_t TimerVector;


//
// -- for the kernel debugger
//    -----------------------
#if IS_ENABLED(KERNEL_DEBUGGER)
    extern Addr_t DebuggerTarget;
#endif



//
// -- some function prototypes
//    ------------------------
static void IdtSetHandler(int i, uint16_t sec, Addr_t *handler, int ist, int dpl);

extern "C" {
    void __attribute__((noreturn)) IdtGenericHandler(ServiceRoutine_t *handler);
    void GsInit(void);
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
typedef struct Idtr_t {
    uint16_t limit;
    Addr_t loc;
} __attribute__((packed)) Idtr_t;
Idtr_t idtr;



//
// -- This is the generic interrupt handler
//    -------------------------------------
void __attribute__((noreturn)) IdtGenericHandler(ServiceRoutine_t *handler)
{
    enum {
        GS  = 0,
        FS  = 1,
        ES  = 2,
        DS  = 3,
        CR4 = 4,
        CR3 = 5,
        CR2 = 6,
        CR0 = 7,
        R15 = 8,
        R14 = 9,
        R13 = 10,
        R12 = 11,
        R11 = 12,
        R10 = 13,
        R9  = 14,
        R8  = 15,
        RDI = 16,
        RSI = 17,
        RBP = 18,
        RDX = 19,
        RCX = 20,
/*      Function_call = 21, */
        RBX = 22,
        RAX = 23,
        INT = 24,
        ERR = 25,
        RIP = 26,
        CS  = 27,
        RFLAGS = 28,
        RSP = 29,
        SS  = 30,
    };

    Addr_t *stack = (Addr_t *)handler->runtimeRegs;
    char buf[200];

    ksprintf(buf, "An exception has occurred on CPU%d by process %-64.64s\n", LapicGetId(),
            CurrentThread()?CurrentThread()->command:"Unknown");
    kprintf(buf);
    kprintf("An error (interrupt %x) has occurred (Error Code %p)\n", stack[INT], stack[ERR]);
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
    kprintf("   FS: %x\t GS: %x\tRFLAGS: %p\n", stack[FS], stack[GS], stack[RFLAGS]);
    kprintf("\n");
    kprintf("CR0: %p\n", stack[CR0]);
    kprintf("CR2: %p\n", stack[CR2]);
    kprintf("CR3: %p\n", stack[CR3]);
    kprintf("CR4: %p\n", stack[CR4]);

    // -- If there is a current Process, block it for cleanup
    if (CurrentThread() != NULL) {
        ProcessTerminate(CurrentThread());
    }


    while (true) {}
}



//
// -- Install the IDT
//    ---------------
#define INT_TARGET(x) int##x
#define INT_VECTOR(x) IdtSetHandler(x, 8, &INT_TARGET(x), 0, 0);
#define ISR_TARGET(x) isr##x
#define ISR_VECTOR(x) IdtSetHandler(x, 8, &ISR_TARGET(x), 0, 0);

void IntInit(void)
{
    INT_VECTOR(0)
    INT_VECTOR(1)
    INT_VECTOR(2)
    INT_VECTOR(3)
    INT_VECTOR(4)
    INT_VECTOR(5)
    INT_VECTOR(6)
    INT_VECTOR(7)
    INT_VECTOR(8)
    INT_VECTOR(9)
    INT_VECTOR(10)
    INT_VECTOR(11)
    INT_VECTOR(12)
    INT_VECTOR(13)
    INT_VECTOR(14)
    INT_VECTOR(15)
    INT_VECTOR(16)
    INT_VECTOR(17)
    INT_VECTOR(18)
    INT_VECTOR(19)
    INT_VECTOR(20)
    INT_VECTOR(21)
    INT_VECTOR(22)
    INT_VECTOR(23)
    INT_VECTOR(24)
    INT_VECTOR(25)
    INT_VECTOR(26)
    INT_VECTOR(27)
    INT_VECTOR(28)
    INT_VECTOR(29)
    INT_VECTOR(30)
    INT_VECTOR(31)

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
    IdtSetHandler(0xe0, 8, &InternalTarget, 0, 0);      // 224

//
// -- handle the kernel debugger, or if none a generic handler
//    --------------------------------------------------------
#if IS_ENABLED(KERNEL_DEBUGGER)
    IdtSetHandler(0xe1, 8, &DebuggerTarget, 0, 0);      // 225
#else
    ISR_VECTOR(225)
#endif

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

    idtr.loc = (Addr_t)idtTable;
    idtr.limit = sizeof(idtTable) - 1;

    __asm__("lidt %0" : : "m"(idtr));

    TssInit();
    GsInit();
}
#undef ISR_VECTOR
#undef ISR_TARGET
#undef INT_VECTOR
#undef INT_TARGET



//
// - Set the address of a new IDT Handler
//   ------------------------------------
static void IdtSetHandler(int i, uint16_t sec, Addr_t *handler, int ist, int dpl)
{
    if (!handler) {
//        IdtSetHandler(i, sec, handler, ist, dpl);
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



