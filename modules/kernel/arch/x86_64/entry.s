;;===================================================================================================================
;;
;;  entry.s -- Entry point for x86_64 kernel
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Based on the multiboot specification, the multiboot loader will hand off control to the following file at the
;;  loader location.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2021-Jan-19  Initial  v0.0.2   ADCL  Initial version
;;
;;===================================================================================================================


                global      entry
                global      TssInit
                global      earlyFrame
                global      loaderInterface
                global      LoadCr3
                global      GetCr3

                extern      kInit
                extern      gdtr
                extern      __bss_start
                extern      _end
                extern      __init_array_start
                extern      __init_array_end
                extern      GsInit


;;
;; -- to handle the 64-bit instruction set, I need to prepare an absolute jmp in memory
;;    ---------------------------------------------------------------------------------
                cpu         x64
                section     .data
                align       8
jmpTgt:
                dw          0x08
                dq          newGdt


earlyFrame:
                dq          0


loaderInterface:
                dq          0


pcid:
                dq          0
                dq          0



;;
;; -- this is the entry point for the kernel
;;    --------------------------------------
                section     .text
entry:
                mov         rax,loaderInterface
                mov         [rax],rdi

                xor         rax,rax
                mov         rcx,_end            ;; -- is this doing too much?
                mov         rax,__bss_start
                mov         rdi,rax
                sub         rcx,rax

                shr         rcx,3
                xor         rax,rax

                rep         stosq


;; -- Determine if we still have any work to do
                mov         rbx,__init_array_start
                mov         rcx,__init_array_end
l1:             cmp         rbx,rcx
                je          e1

;; -- call the initialization function
                push        rbx
                push        rcx
                call        [rbx]
                pop         rcx
                pop         rbx

;; -- move to the next function call
                add         rbx,8
                jmp         l1


;; -- all done initializing
e1:
                mov         rax,gdtr
                lgdt        [rax]


;; -- build an interrupt stack frame for an iret
                mov         rbx,rsp
                xor         rax,rax
                mov         ax,ss

                push        rax
                push        rbx
                pushfq

                mov         rax,0x08
                push        rax
                mov         rax,newGdt
                push        rax
                iretq


newGdt:
;; -- Set the segment selectors for the new GDT
                mov         rax,0x10
                mov         ss,ax

                mov         rax,0x28
                mov         ds,ax
                mov         es,ax

                mov         rax,0
                mov         fs,ax

                call        GsInit

                mov         rax,0x48
                mov         gs,ax

                call        kInit
                jmp         $



;;
;; -- Load the TSS into the register
;;    ------------------------------
TssInit:
                mov         eax,0x50
                ltr         ax
                ret



;;
;; -- Load the new cr3, returning the old value
;;    -----------------------------------------
LoadCr3:
                mov         rax,cr3
                mov         cr3,rdi
                ret



;;
;; -- return the current CR3 value
;;    ----------------------------
GetCr3:
                mov         rax,cr3
                ret

