//====================================================================================================================
//
//  dbg-modules.cc -- Debugging functions and structures for the top level modules to be debugged.
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Oct-12  Initial  v0.0.10  ADCL  Initial version
//
//===================================================================================================================



#include "types.h"
#include "kernel-funcs.h"
#include "heap.h"


//
// -- The list of modules accepting debugging commands
//    ------------------------------------------------
ListHead_t modList = {{0}};


//
// -- This is the state eco-system for debugging the debugger itself
//    --------------------------------------------------------------
static DbgState_t dbgStates[] = {
    {   // main state for the debugger
        .name = "debugger",
        .transitionFrom = 0,
        .transitionTo = 1,
    },
    {
        .function = (Addr_t)DebuggerDumpMods,
    },
};


static DbgTransition_t dbgTransitions[] = {
    {   // -- list the modules known to the debugger
        .command = "modules",
        .alias = "mod",
        .nextState = 1,
    },
    {   // -- exit to the top level menu
        .command = "exit",
        .alias = "x",
        .nextState = -1,
    },
};


DbgModule_t debuggerModule = {
    .name = "debugger",
    .addrSpace = GetAddressSpace(),
    .stack = 0,
    .stateCnt = sizeof(dbgStates) / sizeof(DbgState_t),
    .transitionCnt = sizeof(dbgTransitions) / sizeof(DbgTransition_t),
    .list = {&debuggerModule.list, &debuggerModule.list},
    .lock = {0},
    .states = dbgStates,
    .transitions = dbgTransitions,
};




