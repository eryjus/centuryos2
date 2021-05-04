;;====================================================================================================================
;;
;;  internal-dispatch.h -- dispach an internal function call
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2021-Feb-16  Initial  v0.0.c   ADCL  Initial version (relocated)
;;
;;===================================================================================================================


                global  InternalDispatch
                global  InternalDispatch0
                global  InternalDispatch1
                global  InternalDispatch2
                global  InternalDispatch3
                global  InternalDispatch4
                global  InternalDispatch5
                global  KernelPrintf


;;
;; -- Internal Function 10 -- kprintf
;;    -------------------------------
KernelPrintf:
                push    r9
                push    r8
                push    rcx
                push    rdx
                push    rsi
                push    rdi

                mov     r9,r8
                mov     r8,rcx
                mov     rcx,rdx
                mov     rdx,rsi
                mov     rsi,rdi
                mov     rdi,20
                int     0xe0

                pop     rdi
                pop     rsi
                pop     rdx
                pop     rcx
                pop     r8
                pop     r9

                ret




;;
;; -- dispatch an internal function call
;;    ----------------------------------
InternalDispatch:
InternalDispatch0:
InternalDispatch1:
InternalDispatch2:
InternalDispatch3:
InternalDispatch4:
InternalDispatch5:
                int     0xe0
                ret


