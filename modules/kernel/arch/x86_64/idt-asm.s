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
                global      IdtGenericEntryNoErr
                global      InternalTarget

                extern      IdtGenericHandler
                extern      maxHandlers
                extern      internalTable


                cpu         x64
                section     .text


;;
;; -- This is the entry point for the generic IDT handler
;;    ---------------------------------------------------
IdtGenericEntryNoErr:
                push        qword 0
IdtGenericEntry:
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

                pop         rax                         ;; gs
                pop         rax                         ;; fs
                pop         rax                         ;; es
                pop         rax                         ;; ds

                pop         r15
                pop         r14
                pop         r13
                pop         r12
                pop         r11
                pop         r10
                pop         r9
                pop         r8
                pop         rdi
                pop         rsi
                pop         rbp
                pop         rdx
                pop         rcx
                pop         rbx
                pop         rax

                add         rsp,8                       ;; skip the error code

                iretq


;;
;; -- This is the intenral function handler entry point
;;    -------------------------------------------------
InternalTarget:
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

                mov         rax,maxHandlers
                mov         r15,[rax]
                cmp         rdi,0
                jl          .invalid

                cmp         rdi,r15
                jge         .invalid

                mov         rbp,internalTable
                mov         rbp,[rbp + (rdi * 8)]

                cmp         rbp,0
                je          .none

                call        rbp
                jmp         .out

.invalid:
                mov         eax,-22
                jmp         .out

.none:
                mov         eax,-12

.out:
                pop         r15
                pop         r14
                pop         r13
                pop         r12
                pop         r11
                pop         r10
                pop         r9
                pop         r8
                pop         rdi
                pop         rsi
                pop         rbp
                pop         rdx
                pop         rcx
                pop         rbx
                add         rsp,8

                iretq


