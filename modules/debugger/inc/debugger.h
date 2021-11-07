//===================================================================================================================
//
//  debugger.h -- The header-level definitions and prototypes for the kernel debugger
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Oct-10  Initial  v0.0.10  ADCL  Initial version -- leveraged from the 32-bit code
//
//===================================================================================================================


#pragma once


#include "types.h"


//
// -- Some defines to make the debugger output pretty!
//    ------------------------------------------------
#define     ANSI_PASTE(x)           #x
#define     ANSI_ESC                "\x1b["
#define     ANSI_SET_CURSOR(r,c)    ANSI_ESC ANSI_PASTE(r) ";" ANSI_PASTE(c) "H"
#define     ANSI_CURSOR_UP(x)       ANSI_ESC ANSI_PASTE(x) "A"
#define     ANSI_CURSOR_DOWN(x)     ANSI_ESC ANSI_PASTE(x) "B"
#define     ANSI_CURSOR_FORWARD(x)  ANSI_ESC ANSI_PASTE(x) "C"
#define     ANSI_CURSOR_BACKWARD(x) ANSI_ESC ANSI_PASTE(x) "D"
#define     ANSI_CURSOR_SAVE        ANSI_ESC "s"
#define     ANSI_CURSOR_RESTORE     ANSI_ESC "u"
#define     ANSI_ERASE_LINE         ANSI_ESC "K"
#define     ANSI_CLEAR              ANSI_ESC "2J" ANSI_SET_CURSOR(0,0)

#define     ANSI_ATTR_NORMAL        ANSI_ESC "0m"
#define     ANSI_ATTR_BOLD          ANSI_ESC "1m"
#define     ANSI_ATTR_BLINK         ANSI_ESC "5m"
#define     ANSI_ATTR_REVERSE       ANSI_ESC "7m"

#define     ANSI_FG_BLACK           ANSI_ESC "30m"
#define     ANSI_FG_RED             ANSI_ESC "31m"
#define     ANSI_FG_GREEN           ANSI_ESC "32m"
#define     ANSI_FG_YELLOW          ANSI_ESC "33m"
#define     ANSI_FG_BLUE            ANSI_ESC "34m"
#define     ANSI_FG_MAGENTA         ANSI_ESC "35m"
#define     ANSI_FG_CYAN            ANSI_ESC "36m"
#define     ANSI_FG_WHITE           ANSI_ESC "37m"

#define     ANSI_BG_BLACK           ANSI_ESC "40m"
#define     ANSI_BG_RED             ANSI_ESC "41m"
#define     ANSI_BG_GREEN           ANSI_ESC "42m"
#define     ANSI_BG_YELLOW          ANSI_ESC "43m"
#define     ANSI_BG_BLUE            ANSI_ESC "44m"
#define     ANSI_BG_MAGENTA         ANSI_ESC "45m"
#define     ANSI_BG_CYAN            ANSI_ESC "46m"
#define     ANSI_BG_WHITE           ANSI_ESC "47m"



//
// -- This is a transition in the state machine
//    -----------------------------------------
typedef struct DbgTransition_t {
    char command[DBG_CMD_LEN];
    char alias[DBG_ALIAS_LEN];
    int nextState;
} DbgTransition_t;


//
// -- This is a state in the state machine
//    ------------------------------------
typedef struct DbgState_t {
    char name[DBG_NAME_LEN];
    Addr_t function;
    int transitionFrom;
    int transitionTo;
} DbgState_t;


//
// -- This is the structure to keep track of a module to be debugged
//    --------------------------------------------------------------
typedef struct DbgModule_t {
    char name[MOD_NAME_LEN];              // this will be the name from the module header; capped at 16 char
    Addr_t addrSpace;
    Addr_t stack;
    size_t stateCnt;
    size_t transitionCnt;
    ListHead_t::List_t list;
    Spinlock_t lock;
    DbgState_t *states;
    DbgTransition_t *transitions;
} DbgModule_t;


//
// -- This is the list of modules
//    ---------------------------
extern ListHead_t modList;
extern DbgModule_t debuggerModule;



//
// -- Function prototypes
//    -------------------
extern "C" {
    Return_t dbg_Output(const char *str);
    void DebuggerMain(void);
    void DebuggerCall(Addr_t addrSpace, Addr_t function, Addr_t stack);
    bool DbgSerialHasChar(void);
    uint8_t DbgSerialGetChar(void);
    void DbgSerialPutChar(uint8_t ch);
    void DbgSerialPutString(const char *s);

    void DebuggerDumpMods(void);
    void DebuggerEngage(void);
    void DebuggerRelease(void);
}


