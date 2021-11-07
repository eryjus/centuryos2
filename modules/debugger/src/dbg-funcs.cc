//====================================================================================================================
//
//  dbg-funcs.cc -- Debugging functions structures and management functions
//
//        Copyright (c)  2017-2021 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This source file handles the debugger function tables.  It is intended that the debugger interface will accept
//  the name of a module and a command supported by that module, such as:
//
//  `pmm status`
//
//  This module would then search the table for an entry matching the module "pmm" and the command "status" and
//  then execute the debugging function.  I would need to get into the kernel to swap out the address space to the
//  actual module address space.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2021-Oct-10  Initial  v0.0.10  ADCL  Initial version
//
//===================================================================================================================



#include "types.h"
#include "lists.h"
#include "boot-interface.h"
#include "kernel-funcs.h"
#include "heap.h"
#include "stacks.h"



//
// -- Here are the local function prototypes
//    --------------------------------------
extern "C" {
    Return_t dbg_Register(DbgModule_t *mod, DbgState_t *states, DbgTransition_t *transitions);
    char *dbg_GetResponse(void);
    Return_t dbg_Installed(void);

    Return_t dbg_Dispatch(Addr_t *reg);

    Return_t DebuggerEarlyInit(BootInterface_t *loader);
    void DebuggerLateInit(void);
    void IpiHandleDebugger(Addr_t *regs);
}



//
// -- This structure is the definition of a function that can be called
//    -----------------------------------------------------------------
typedef struct DebugFunction_t {
    char *module;
    char *name;
    Addr_t addrSpace;
    Addr_t address;
    char *shortDesc;
    char *help;
    ListHead_t::List_t list;
} DebugFunction_t;



//
// -- This is the list of supported functions
//    ---------------------------------------
static uint16_t base = 0x3f8;



//
// -- Output a single character to the serial port
//    --------------------------------------------
void DbgSerialPutChar(uint8_t ch)
{
    if (ch == '\n') DbgSerialPutChar('\r');

    while ((INB(base + 5) & 0x20) == 0) {}

    OUTB(base + 0, ch);
}



//
// -- Output a string of characters to the serial port
//    ------------------------------------------------
void DbgSerialPutString(const char *s)
{
    while (*s) {
        DbgSerialPutChar(*s ++);
    }
}



//
// -- Does the serial port have a character to read
//    ---------------------------------------------
bool DbgSerialHasChar(void)
{
    return ((INB(base + 5) & 1) != 0);
}



//
// -- get a character from the serial port
//    ------------------------------------
uint8_t DbgSerialGetChar(void)
{
    while (!DbgSerialHasChar()) {}
    return INB(base);
}



//
// -- Print the debugger help information
//    -----------------------------------
void DebuggerHelp(const char *module)
{
#if 0
    ListHead_t::List_t *wrk = dbgFuncList.list.next;
    static char buf[256];

    kMemMoveB(buf, 0, sizeof(buf));

    while (wrk != &dbgFuncList.list) {
        DebugFunction_t *func = FIND_PARENT(wrk, DebugFunction_t, list);

        ksprintf(buf, "%s - %s\t%s\n", func->module, func->name, func->shortDesc);

        wrk = wrk->next;
    }
#endif
}


//
// -- Output a debugging string to the serial port
//    --------------------------------------------
Return_t dbg_Output(const char *str)
{
    DbgSerialPutString(str);
    DbgSerialPutString(ANSI_ATTR_NORMAL);

    return 0;
}



//
// -- Register a new debugger function, updating an existing if required
//    ------------------------------------------------------------------
Return_t dbg_Register(DbgModule_t *mod, DbgState_t *states, DbgTransition_t *transitions)
{
    if (!assert(mod != NULL)) return -EINVAL;
    if (!assert(states != NULL)) return -EINVAL;
    if (!assert(transitions != NULL)) return -EINVAL;

    SpinLock(&modList.lock); {
        ListInit(&mod->list);

        mod->states = states;
        mod->transitions = transitions;

        ListAddTail(&modList, &mod->list);
    } SpinUnlock(&modList.lock);

    return 0;
}


//
// -- Initialize the debugger tables
//    ------------------------------
Return_t DebuggerEarlyInit(BootInterface_t *loader)
{
#if IS_ENABLED(KERNEL_DEBUGGER)
    extern Addr_t __stackSize;

    ProcessInitTable();

    debuggerModule.stack = StackFind();
    for (Addr_t s = debuggerModule.stack; s < debuggerModule.stack + __stackSize; s += PAGE_SIZE) {
        MmuMapPage(s, PmmAlloc(), PG_WRT);
    }
    debuggerModule.stack += __stackSize;

    ListInit(&modList.list);

    // -- no lock required here -- still single threaded
    ListAdd(&modList, &debuggerModule.list);

    return 0;

#else

    return -EINVAL;

#endif
}



//
// -- return that we have a debugger installed
//    ----------------------------------------
Return_t dbg_Installed(void)
{
    return true;
}



//
// -- Perform the late init procedure, creating the debugger process
//    --------------------------------------------------------------
void DebuggerLateInit(void)
{
    KernelPrintf("Starting Debugger Process\n");
    SchProcessCreate("Debugger", (Addr_t)DebuggerMain, GetAddressSpace(), PTY_OS);
}



Return_t dbg_Dispatch(Addr_t *reg)
{
    return -EINVAL;
}


char *dbg_GetResponse(void)
{
    return NULL;
}


void DebuggerDumpMods(void)
{
    dbg_Output(ANSI_CLEAR ANSI_SET_CURSOR(0,0));
    dbg_Output(ANSI_FG_RED ANSI_ATTR_BOLD "List Known Debugger Modules\n");

    ListHead_t::List_t *wrk = modList.list.next;

    dbg_Output("+----------------------+----------+-------------+-----------+\n");
    dbg_Output("| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Module Name" ANSI_ATTR_NORMAL
            "          | " ANSI_ATTR_BOLD ANSI_FG_BLUE "States" ANSI_ATTR_NORMAL
            "   | " ANSI_ATTR_BOLD ANSI_FG_BLUE "Transitions" ANSI_ATTR_NORMAL " | "
            ANSI_ATTR_BOLD ANSI_FG_BLUE "Functions" ANSI_ATTR_NORMAL " |\n");
    dbg_Output("+----------------------+----------+-------------+-----------+\n");

    while (wrk != &modList.list) {
        char buf[256];

        DbgModule_t *mod = FIND_PARENT(wrk, DbgModule_t, list);

        int fc = 0;
        for (int i = 0; i < mod->stateCnt; i ++) {
            if (mod->states[i].function) fc ++;
        }

        ksprintf(buf, "| " ANSI_ATTR_BOLD "%-20.20s" ANSI_ATTR_NORMAL " | %8d | %11d | %9d |\n",
                mod->name,
                mod->stateCnt,
                mod->transitionCnt,
                fc);

        dbg_Output(buf);

        wrk = wrk->next;
    }

    dbg_Output("+----------------------+----------+-------------+-----------+\n");
}






