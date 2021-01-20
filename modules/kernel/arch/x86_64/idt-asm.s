;;===================================================================================================================
;;
;;  idt-asm.cc -- assembly-level interfacing to the IDT
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2021-Jan-13  Initial  v0.0.2   ADCL  Initial version
;;
;;===================================================================================================================


                global      IdtGenericEntry

                extern      IdtGenericHandler


                section     .text


;;
;; -- This is the entry point for the generic IDT handler
;;    ---------------------------------------------------
IdtGenericEntry:
                mov         eax,0x12345678
                push        rax
                push        rbx
                push        rcx
                push        rdx
                push        rbp
                push        rsi
                push        rdi
                push        r8
                push        r9
                push        r10
                push        r11
                push        r12
                push        r13
                push        r14
                push        r15

                xor         rax,rax
                mov         ax,ds
                push        rax

                mov         ax,es
                push        rax

                mov         ax,fs
                push        rax

                mov         ax,gs
                push        rax

                mov         rdi,rsp                     ;; the pionter to the stack containing the variables
                sub         rdi,8
                call        IdtGenericHandler
                iret

