;;===================================================================================================================
;;
;;  entry.s -- Entry point for x86_64 architecture
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2021-Oct-10  Initial  v0.0.10  ADCL  Initial version
;;
;;===================================================================================================================


        global  header
        global  DebuggerCall

        extern  DebuggerEarlyInit
        extern  DebuggerLateInit
        extern  dbg_Installed
        extern  dbg_Register
        extern  dbg_GetResponse
        extern  dbg_Output

        extern  dbg_Dispatch
        extern  dbg_PromptGeneric
        extern  IpiHandleDebugger


%include        'constants.inc'

        section .text


;;
;; -- Set up the header structure for parsing from the kernel
;;    -------------------------------------------------------
header:
        db      'C','e','n','t','u','r','y',' ','O','S',' ','6','4',0,0,0   ;; Sig
        db      'D','E','B','U','G','G','E','R',0,0,0,0,0,0,0,0             ;; Name
        dq      DebuggerEarlyInit                                           ;; Early Init
        dq      DebuggerLateInit                                            ;; Late Init
        dq      0xffffaf4000000000                                          ;; Stack Locations
        dq      1                                                           ;; interrupts
        dq      4                                                           ;; internal Services
        dq      0                                                           ;; OS services
        dq      0xe1                                                        ;; Interrupt 1
        dq      dbg_Dispatch                                                ;; .. target address
        dq      0                                                           ;; .. stack
        dq      0x070                                                       ;; internal function 1
        dq      dbg_Installed                                               ;; .. target address
        dq      0                                                           ;; .. stack
        dq      0x071                                                       ;; internal function 2
        dq      dbg_Register                                                ;; .. target address
        dq      0                                                           ;; .. stack
        dq      0x072                                                       ;; internal function 3
        dq      dbg_Output                                                  ;; .. target address
        dq      0                                                           ;; .. stack
        dq      0x073                                                       ;; internal function 4
        dq      dbg_PromptGeneric                                           ;; .. target address
        dq      0                                                           ;; .. stack




;;
;; -- Handle calling the actual function to handle the debugging request
;;
;;    Prototype: void DebuggerCall(Addr_t p1, Addr_t p2, Addr_t p3, Addr_t addrSpace, Addr_t function, Addr_t stack);
;;    ---------------------------------------------------------------------------------------------------------------
DebuggerCall:
        int     0xe1
        ret

