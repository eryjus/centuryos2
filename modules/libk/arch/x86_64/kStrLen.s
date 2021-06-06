;;===================================================================================================================
;;
;;  kStrLen.s -- calculate the length of a string
;;
;;        Copyright (c)  2017-2021 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Calculate the length of a string, returning the result in ax
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2021-May-25  Initial  v0.0.9b  ADCL  Initial version
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global      kStrLen


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
    section     .text
    bits        64


;;
;; -- Calculate the length of the string
;;    ----------------------------------
kStrLen:
    cld
    push    rcx                                                 ;; save this register
    push    rdi                                                 ;; and this one

    mov     rcx,-1                                              ;; set max chars to scan (lots)
    xor     rax,rax                                             ;; al holds the char to find (NULL)

    repne   scasb                                               ;; find '\0', decrementing ecx as you go

    not     rcx                                                 ;; where did we start - take complement
    mov     rax,rcx                                             ;; move it to return
    dec     rax                                                 ;; and subtract one

    pop     rdi                                                 ;; restore the register
    pop     rcx                                                 ;; and this one
    ret
