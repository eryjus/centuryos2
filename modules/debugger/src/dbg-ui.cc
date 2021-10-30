/****************************************************************************************************************//**
*   @file               dbg-ui.cc
*   @brief              Implements the user interface for the debugger -- including the main debugger process itself
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Oct-10
*   @since              v0.0.10
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*   The debugger is implemented as a finite state machine -- well, a collection of independent state machines.
*   At the top level, the debugger will prompt for which module to debug.  There are no paths from one module
*   to another.  Therefore, a state is represented as a tuple of the module pointer and the state number within
*   that module.
*
*   Transisions from one state to another are all held in an array and the transitions from any state should be
*   indexed into the array as sequential elements.
*
*   It is the purpose of this file to handle reading input from the user against the possible states and handle
*   things gracefully.  Even with incorrect input.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Oct-10 | Initial |  v0.0.10 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "kernel-funcs.h"



/****************************************************************************************************************//**
*   @struct             DbgFullState_t
*   @brief              The full state of the debugger, with its current module and its state.
*
*   A "state" in this debugger is represented by the module it is operating in and the state within that module.
*   This structure is used to communicate that state around the system.
*
*   @note               This structure is passed around by value, not by pointer or reference.
*///-----------------------------------------------------------------------------------------------------------------
struct DbgFullState_t {
    DbgModule_t     *mod;       //!< The module being debugged.
    int             state;      //!< The current state within the module.
};


/****************************************************************************************************************//**
*   @typedef            DbgFullState_t
*   @brief              Type formalization of the full state of the debugger.
*
*   @see                struct DbgFullState_t
*
*   @note               This type is passed around by value, not by pointer or reference.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct DbgFullState_t DbgFullState_t;




/****************************************************************************************************************//**
*   @var                dbgCmd
*   @brief              The global command buffer.
*///-----------------------------------------------------------------------------------------------------------------
char dbgCmd[DBG_MAX_CMD_LEN];



//*******************************************************************************************************************
//  Local Function prototypes
//-------------------------------------------------------------------------------------------------------------------
extern "C" Return_t dbg_PromptGeneric(int, const char *prompt, char *result, size_t size);

static void DebuggerGetInput(void);
static void DebuggerIsolateCommand(void);
static DbgFullState_t DebuggerParseModule(void);
static DbgFullState_t DebuggerParseState(DbgFullState_t state);
static void DebuggerPromptState(DbgFullState_t state);
static void DebuggerPromptTopLevel(void);

void DebuggerMain(void);



/****************************************************************************************************************//**
*   @fn                 static void DebuggerIsolateCommand(void)
*   @brief              Isolate one command in dbgCmd, terminating it with '\0'.
*
*   Finds the first space character in \ref dbgCmd and replaces that with a '\0'.  It does not otherwise alter the
*   remainder of the buffer.  If there are no spaces, \ref dbgCmd is unchanged.
*///-----------------------------------------------------------------------------------------------------------------
static void DebuggerIsolateCommand(void)
{
    char *c = dbgCmd;
    char *e = dbgCmd + DBG_MAX_CMD_LEN;

    while (true && c < e && *c) {
        if (*c == ' ') {
            *c = 0;
            return;
        }
        c ++;
    }
}



/****************************************************************************************************************//**
*   @fn                 static void DebuggerNextCommand(void)
*   @brief              Move the \ref dbgCmd input to the next command
*
*   Moves any pending commands up in the \ref dbgCmd buffer.
*///-----------------------------------------------------------------------------------------------------------------
static void DebuggerNextCommand(void)
{
    // -- move any additional command up
    int len = kStrLen(dbgCmd);
    kMemMoveB(dbgCmd, dbgCmd + len + 1, kStrLen(dbgCmd + len + 1) + 1);
    kMemSetB(dbgCmd + kStrLen(dbgCmd), 0, DBG_MAX_CMD_LEN - kStrLen(dbgCmd));
}




/****************************************************************************************************************//**
*   @fn                 void DebuggerMain(void)
*   @brief              This is the main debugger process loop.
*
*   This never ending loop issues prompts and accepts input from the user based on the state of the debugger.  The
*   state is identified by \ref DbgFullState_t.  This is the entry point for the debugger process.
*///-----------------------------------------------------------------------------------------------------------------
void DebuggerMain(void)
{
    dbg_Output(0, ANSI_CLEAR ANSI_SET_CURSOR(0,0) ANSI_FG_RED ANSI_ATTR_BOLD
            "Welcome to the Century-OS kernel debugger\n");

    // -- set the current state to be no module and the starting state
    DbgFullState_t currentState = {NULL, 0};
    DbgFullState_t nextState = {NULL, 0};
    dbgCmd[0] = 0;

    while (true) {

#if DEBUG_ENABLED(DebuggerMain)
        KernelPrintf("\n\n[Current Command]: %s\n", dbgCmd);
#endif

        if (currentState.mod == NULL) {
            DebuggerPromptTopLevel();
            nextState = DebuggerParseModule();
        } else {
            DebuggerPromptState(currentState);
            nextState = DebuggerParseState(currentState);
        }

        if (nextState.state == -1 || nextState.mod == NULL) {
            currentState.mod = NULL;
            currentState.state = 0;
            dbgCmd[0] = 0;
            continue;
        }

        if (nextState.mod->states[nextState.state].function != 0) {
            Addr_t flags = KrnPauseCores();
            DebuggerCall(nextState.mod->addrSpace,
                    nextState.mod->states[nextState.state].function,
                    nextState.mod->stack);
            KrnReleaseCores(flags);
            dbgCmd[0] = 0;
            continue;
        }

        currentState = nextState;
    }
}



/****************************************************************************************************************//**
*   @fn                 void DebuggerPromptState(DbgFullState_t state)
*   @brief              While in a module, prompt the current state for the next transition.
*
*   @param[in]          state       The current state
*
*   @see                DbgFullState_t
*
*   @redmine{501}       ~~DebuggerPromptState is not checking bounds~~
*
*   When in a module and there is no remaining input to handle, issue the prompts for the current state to the
*   user.  This function will kindly offer all the possible options to the user below the current command line.
*///-----------------------------------------------------------------------------------------------------------------
static void DebuggerPromptState(DbgFullState_t state)
{
    if (dbgCmd[0]) return;          // -- outstanding commands to process

    assert(state.mod != NULL);
    assert(state.state < state.mod->stateCnt);
    assert(state.mod->states[state.state].transitionFrom <= state.mod->states[state.state].transitionTo);
    assert(state.mod->states[state.state].transitionTo < state.mod->transitionCnt);

    bool first = true;

    dbg_Output(0, "\n (allowed: ");

    for (int i = state.mod->states[state.state].transitionFrom;
            i <= state.mod->states[state.state].transitionTo;
            i ++) {
        if (first) first = false;
        else dbg_Output(0, ", ");

        dbg_Output(0, state.mod->transitions[i].command);
    }

    dbg_Output(0, ")\r" ANSI_CURSOR_UP(1));
    dbg_Output(0, state.mod->states[state.state].name);
    dbg_Output(0, " :> ");
}



/****************************************************************************************************************//**
*   @fn                 DbgFullState_t DebuggerParseState(DbgFullState_t state)
*   @brief              Parse the input for the proper next transition while in a module.
*
*   @param[in]          state       The current state
*
*   @returns            The next state for the state machine.
*
*   @see                DbgFullState_t
*
*   @redmine{502}       ~~DebuggerParseState is not checking bounds~~
*
*   Reviews the possible transitions from the current state with the input in \ref dbgCmd and when a match is
*   found returns the next state indicated on the transition node.  If no input is pending, this functions calls
*   \ref DebuggerGetInput to get more data in \ref dbgCmd.  If there is an error in parsing the input, then the
*   contents of \ref dbgCmd are cleared and, after issuing an error message, the current state is returned
*   for re-prompt.
*///-----------------------------------------------------------------------------------------------------------------
static DbgFullState_t DebuggerParseState(DbgFullState_t state)
{
    DbgFullState_t rv = state;
    bool goodInput = false;

    if (dbgCmd[0] == 0) DebuggerGetInput();

    DebuggerIsolateCommand();

    assert(state.mod != NULL);
    assert(state.state < state.mod->stateCnt);
    assert(state.mod->states[state.state].transitionFrom <= state.mod->states[state.state].transitionTo);
    assert(state.mod->states[state.state].transitionTo < state.mod->transitionCnt);

    for (int i = state.mod->states[state.state].transitionFrom;
            i <= state.mod->states[state.state].transitionTo;
            i ++) {
        if (kStrCmp(state.mod->transitions[i].command, dbgCmd) == 0
                || kStrCmp(state.mod->transitions[i].alias, dbgCmd) == 0) {
            if (state.mod->transitions->nextState < 0) {
                return rv;
            }

            assert(state.mod->transitions[i].nextState == -1
                    || state.mod->transitions[i].nextState < state.mod->stateCnt);

            rv.state = state.mod->transitions[i].nextState;
            rv.mod = state.mod;
            goodInput = true;
            break;
        }
    }

    if (!goodInput) {
        dbg_Output(0, ANSI_FG_RED "Invalid Input\n");
        dbgCmd[0] = 0;
    }

    DebuggerNextCommand();

    return rv;
}



/****************************************************************************************************************//**
*   @fn                 void DebuggerPromptTopLevel(void)
*   @brief              Issues a prompt for which module to enter for debugging.
*
*   At the top level of the tree, issue the prompts for which module to enter to the user.  This function will
*   kindly offer all the possible options to the user below the current command line.
*///-----------------------------------------------------------------------------------------------------------------
static void DebuggerPromptTopLevel(void)
{
    bool first = true;
    dbg_Output(0, "\n (modules: ");

    ListHead_t::List_t *wrk = modList.list.next;

    while (wrk != &modList.list) {
        DbgModule_t *mod = FIND_PARENT(wrk, DbgModule_t, list);

        if (first) first = false;
        else dbg_Output(0, ", ");

        dbg_Output(0, mod->name);

        wrk = wrk->next;
    }

    dbg_Output(0, ")\r" ANSI_CURSOR_UP(1) "- :> ");
}



/****************************************************************************************************************//**
*   @fn                 void DebuggerGetInput(void)
*   @brief              Gets more input from the user, placing the characters entered into \ref dbgCmd.
*
*   Accepts input from the user and places the input into the \ref dbgCmd buffer.  All upper-case input is
*   down-cased to lower case.  No effort is made to parse the input, just collect it.
*
*   @redmine{495}       The Debugger process is hogging CPU
*///-----------------------------------------------------------------------------------------------------------------
static void DebuggerGetInput(void)
{
    kMemSetB(dbgCmd, 0, DBG_MAX_CMD_LEN);
    int pos = 0;

    while (true) {
        // -- we always need room for at least one more character
        if (pos == DBG_MAX_CMD_LEN - 1) {
            pos = DBG_MAX_CMD_LEN - 2;
        }

        char ch = (char)DbgSerialGetChar();

        // -- an enter completes the command input
        if (ch == 13 || ch == 10) {
            if (*dbgCmd == 0) continue;       // make sure we do not have an empty buffer
            DbgSerialPutString("\n" ANSI_ERASE_LINE);
            return;
        } else if (ch < ' ') continue;              // other special characters ignored
        else if (ch == 127) {
            if (--pos < 0) {
                pos = 0;
                continue;
            }
            DbgSerialPutString("\b \b");
            dbgCmd[pos] = '\0';
            continue;
        }

        // -- downcase any upper case letter
        if (ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 'a';

        dbgCmd[pos ++] = ch;
        DbgSerialPutChar(ch);
    }
}



/****************************************************************************************************************//**
*   @fn                 DbgFullState_t DebuggerParseModule(void)
*   @brief              Parse the input for the proper module to enter for debugging.
*
*   @returns            The entry state for the state machine.
*
*   @see                DbgFullState_t
*
*   Reviews the possible modules with the input in \ref dbgCmd and when a match is found returns the entry state
*   for the module.  The entry state is state number 0.  If no input is pending, this functions calls
*   \ref DebuggerGetInput to get more data in \ref dbgCmd.  If there is an error in parsing the input, then the
*   contents of \ref dbgCmd are cleared and, after issuing an error message, the top-level state is returned, which
*   is a NULL module and 0 state.
*///-----------------------------------------------------------------------------------------------------------------
static DbgFullState_t DebuggerParseModule(void)
{
    DbgFullState_t rv = {NULL, 0};
    ListHead_t::List_t *wrk = modList.list.next;
    bool goodInput = false;

    if (dbgCmd[0] == 0) DebuggerGetInput();

    DebuggerIsolateCommand();

    // -- scan for the command entered
    while (wrk != &modList.list) {
        DbgModule_t *mod = FIND_PARENT(wrk, DbgModule_t, list);

        if (kStrCmp(mod->name, dbgCmd) == 0) {
            rv.state = 0;
            rv.mod = mod;
            goodInput = true;
            break;
        }

        wrk = wrk->next;
    }


    if (!goodInput) {
        dbg_Output(0, ANSI_FG_RED "Invalid Input\n");
        dbgCmd[0] = 0;
    }

    DebuggerNextCommand();

    return rv;
}



/****************************************************************************************************************//**
*   @fn                 Return_t dbg_PromptGeneric(int, const char *prompt, char *result, size_t size)
*   @brief              Generic prompt for input and return that back to the caller
*
*   @returns            Success of failure of getting the input
*   @retval             SUCCESS         When the value in the buffer was successfully collected
*   @retval             -EINVAL         When the prompt is NULL\n
*                                       When the target response buffer is NULL\n
*                                       When the size is <= 0
*
*   Issues a generic prompt (provided by the calling function) and accepts input in the form of a string.  The input
*   is then size adjusted (as required) to fit the target buffer and then copies into the buffer supplied.
*///-----------------------------------------------------------------------------------------------------------------
Return_t dbg_PromptGeneric(int, const char *prompt, char *result, size_t size)
{
    if (!assert(prompt != NULL)) return -EINVAL;
    if (!assert(result != NULL)) return -EINVAL;
    if (!assert(size > 0)) return -EINVAL;

    char buf[60];

    if (dbgCmd[0] == 0) {
        ksprintf(buf, "%s :> ", prompt);
        dbg_Output(0, buf);
        DebuggerGetInput();
    }

    DebuggerIsolateCommand();

    char sv = 0;
    if (kStrLen(dbgCmd) > size - 1) {
        sv = dbgCmd[size - 1];
        dbgCmd[size - 1] = 0;
    }

    kStrCpy(result, dbgCmd);

    if (sv) dbgCmd[size - 1] = sv;

    DebuggerNextCommand();

    return SUCCESS;
}
