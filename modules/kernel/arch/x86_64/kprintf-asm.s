;;===================================================================================================================
;;
;;  kprintf-asm.cc -- assembly-level interfacing for KernelPrintf(), which manages the variable parameters
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2021-Sep-11  Initial  v0.0.9d  ADCL  Initial version
;;
;;===================================================================================================================


;;
;; -- Expose and declare some addresses
;;    ---------------------------------
        global  krn_KernelPrintf

        extern  kprintf


;;
;; -- massage the parameter config back to the proper state
;;    -----------------------------------------------------
krn_KernelPrintf:
        mov     rdi,rsi
        mov     rsi,rdx
        mov     rdx,rcx
        mov     rcx,r8
        mov     r8,r9
        mov     r9,r11
        jmp     kprintf
